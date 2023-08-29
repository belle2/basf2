##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
"""
This module implements the IEA-GAN generator model.
"""

import functools
import json
import math

import torch
from torch import nn
from torch import optim
from torch.nn import init
from torch.nn import Parameter as P
import torch.nn.functional as F


CONFIG = json.loads(
    """{
    "num_workers": 8,
    "seed": 415,
    "pin_memory": false,
    "shuffle": true,
    "augment": 0,
    "use_multiepoch_sampler": false,
    "model": "BigGAN_deep",
    "G_ch": 32,
    "G_param" : "SN",
    "D_param" : "SN",
    "D_ch": 32,
    "G_depth": 2,
    "D_depth": 2,
    "H_base": 3,
    "D_wide": true,
    "G_shared": true,
    "shared_dim": 128,
    "dim_z": 128,
    "z_var": 1.0,
    "hier": true,
    "cross_replica": false,
    "mybn": false,
    "G_activation": "inplace_relu",
    "D_activation": "inplace_relu",
    "G_attn": "0",
    "D_attn": "0",
    "norm_style": "bn",
    "G_init": "ortho",
    "D_init": "ortho",
    "skip_init": false,
    "G_lr": 5e-05,
    "D_lr": 5e-05,
    "G_B1": 0.0,
    "D_B1": 0.0,
    "G_B2": 0.999,
    "D_B2": 0.999,
    "batch_size": 40,
    "G_batch_size": 0,
    "num_G_accumulations": 1,
    "num_D_steps": 1,
    "num_D_accumulations": 1,
    "split_D": true,
    "num_epochs": 4,
    "parallel": false,
    "G_fp16": false,
    "D_fp16": false,
    "D_mixed_precision": false,
    "G_mixed_precision": false,
    "accumulate_stats": false,
    "num_standing_accumulations": 16,
    "G_eval_mode": true,
    "save_every": 1000,
    "test_every": 1000,
    "num_save_copies": 2,
    "num_best_copies": 2,
    "ema": true,
    "ema_decay": 0.9999,
    "use_ema": true,
    "ema_start": 10000,
    "adam_eps": 1e-06,
    "BN_eps": 1e-05,
    "SN_eps": 1e-06,
    "num_G_SVs": 1,
    "num_D_SVs": 1,
    "num_G_SV_itrs": 1,
    "num_D_SV_itrs": 1,
    "G_ortho": 0.0001,
    "D_ortho": 0.0,
    "toggle_grads": true,
    "logstyle": "%3.3e",
    "sv_log_interval": 10,
    "log_interval": 100,
    "resolution": 256,
    "n_classes": 40,
    "run_name": "BGd_140",
    "resume": false,
    "latent_op": false,
    "latent_reg_weight": 300,
    "bottom_width": 4,
    "add_blur" : false,
    "add_noise": true,
    "add_style": false,
    "conditional_strategy": "Contra",
    "hypersphere_dim": 1024,
    "pos_collected_numerator": false,
    "nonlinear_embed": false,
    "normalize_embed": true,
    "inv_stereographic" :false,
    "contra_lambda": 1.0,
    "Angle": false,
    "angle_lambda": 1.0,
    "IEA_loss": true,
    "IEA_lambda": 1.0,
    "Uniformity_loss": true,
    "unif_lambda": 0.1,
    "diff_aug": true,
    "Con_reg": false,
    "cr_lambda": 10,
    "pixel_reg": false,
    "px_lambda": 1.0,
    "RRM_prx_G": true,
    "normalized_proxy_G": false,
    "RRM_prx_D": false,
    "RRM_embed": true,
    "n_head_G": 2,
    "n_head": 4,
    "attn_type": "sa",
    "sched_version": "default",
    "z_dist": "normal",
    "truncated_threshold": 1.0,
    "clip_norm": "None",
    "amsgrad": false,
    "arch": "None",
    "G_kernel_size": 3,
    "D_kernel_size": 3,
    "ada_belief": false,
    "pbar": "tqdm",
    "which_best": "FID",
    "stop_after": 100000,
    "trunc_z": 0.5,
    "denoise": false,
    "metric_log_name": "metric_log.jsonl",
    "reinitialize_metric_logs": false,
    "reinitialize_parameter_logs": false,
    "num_incep_images": 16000,
    "load_optim": true}"""
)


def proj(x, y):
    """
    Projection of x onto y
    """
    return torch.mm(y, x.t()) * y / torch.mm(y, y.t())


def gram_schmidt(x, ys):
    """
    Orthogonalize x wrt list of vectors ys
    """
    for y in ys:
        x = x - proj(x, y)
    return x


def power_iteration(W, u_, update=True, eps=1e-12):
    """
    Apply num_itrs steps of the power method to estimate top N singular values.
    """
    # Lists holding singular vectors and values
    us, vs, svs = [], [], []
    for i, u in enumerate(u_):
        # Run one step of the power iteration
        with torch.no_grad():
            v = torch.matmul(u, W)
            # Run Gram-Schmidt to subtract components of all other singular vectors  # noqa
            v = F.normalize(gram_schmidt(v, vs), eps=eps)
            # Add to the list
            vs += [v]
            # Update the other singular vector
            u = torch.matmul(v, W.t())
            # Run Gram-Schmidt to subtract components of all other singular vectors  # noqa
            u = F.normalize(gram_schmidt(u, us), eps=eps)
            # Add to the list
            us += [u]
            if update:
                u_[i][:] = u
        # Compute this singular value and add it to the list
        svs += [torch.squeeze(torch.matmul(torch.matmul(v, W.t()), u.t()))]
        # svs += [torch.sum(F.linear(u, W.transpose(0, 1)) * v)]
    return svs, us, vs


def groupnorm(x, norm_style):
    """
    Simple function to handle groupnorm norm stylization
    """
    # If number of channels specified in norm_style:
    if "ch" in norm_style:
        ch = int(norm_style.split("_")[-1])
        groups = max(int(x.shape[1]) // ch, 1)
    # If number of groups specified in norm style
    elif "grp" in norm_style:
        groups = int(norm_style.split("_")[-1])
    # If neither, default to groups = 16
    else:
        groups = 16
    return F.group_norm(x, groups)


class identity(nn.Module):
    """
    Convenience passthrough function
    """

    #: forward
    def forward(self, tensor: torch.Tensor):
        return tensor


class SN(object):
    """
    Spectral normalization base class
    """
    # pylint: disable=no-member

    def __init__(self, num_svs, num_itrs, num_outputs, transpose=False, eps=1e-12):
        """constructor"""

        #: Number of power iterations per step
        self.num_itrs = num_itrs
        #: Number of singular values
        self.num_svs = num_svs
        #: Transposed?
        self.transpose = transpose
        #: Epsilon value for avoiding divide-by-0
        self.eps = eps
        # Register a singular vector for each sv
        for i in range(self.num_svs):
            self.register_buffer(f"u{i:d}", torch.randn(1, num_outputs))
            self.register_buffer(f"sv{i:d}", torch.ones(1))

    @property
    def u(self):
        """
        Singular vectors (u side)
        """
        return [getattr(self, f"u{i:d}") for i in range(self.num_svs)]

    @property
    def sv(self):
        """
        Singular values
        note that these buffers are just for logging and are not used in training.
        """
        return [getattr(self, f"sv{i:d}") for i in range(self.num_svs)]

    def W_(self):
        """
        Compute the spectrally-normalized weight
        """
        W_mat = self.weight.view(self.weight.size(0), -1)
        if self.transpose:
            W_mat = W_mat.t()
        # Apply num_itrs power iterations
        for _ in range(self.num_itrs):
            svs, _, _ = power_iteration(
                W_mat, self.u, update=self.training, eps=self.eps
            )
            # Update the svs
        if self.training:
            # Make sure to do this in a no_grad() context or you'll get memory leaks!  # noqa
            with torch.no_grad():
                for i, sv in enumerate(svs):
                    self.sv[i][:] = sv
        return self.weight / svs[0]


class SNConv2d(nn.Conv2d, SN):
    """
    2D Conv layer with spectral norm
    """

    #: Constructor
    def __init__(
        self,
        in_channels,
        out_channels,
        kernel_size,
        stride=1,
        padding=0,
        dilation=1,
        groups=1,
        bias=True,
        num_svs=1,
        num_itrs=1,
        eps=1e-12,
    ):
        nn.Conv2d.__init__(
            self,
            in_channels,
            out_channels,
            kernel_size,
            stride,
            padding,
            dilation,
            groups,
            bias,
        )
        SN.__init__(self, num_svs, num_itrs, out_channels, eps=eps)

    #: forward
    def forward(self, x):
        return F.conv2d(
            x,
            self.W_(),
            self.bias,
            self.stride,
            self.padding,
            self.dilation,
            self.groups,
        )


class SNLinear(nn.Linear, SN):
    """
    Linear layer with spectral norm
    """

    #: Constructor
    def __init__(
        self,
        in_features,
        out_features,
        bias=True,
        num_svs=1,
        num_itrs=1,
        eps=1e-12,
    ):
        nn.Linear.__init__(self, in_features, out_features, bias)
        SN.__init__(self, num_svs, num_itrs, out_features, eps=eps)

    #: forward
    def forward(self, x):
        return F.linear(x, self.W_(), self.bias)


def fused_bn(x, mean, var, gain=None, bias=None, eps=1e-5):
    """Fused batchnorm op"""

    # Apply scale and shift--if gain and bias are provided, fuse them here
    # Prepare scale
    scale = torch.rsqrt(var + eps)
    # If a gain is provided, use it
    if gain is not None:
        scale = scale * gain
    # Prepare shift
    shift = mean * scale
    # If bias is provided, use it
    if bias is not None:
        shift = shift - bias
    return x * scale - shift
    # return ((x - mean) / ((var + eps) ** 0.5)) * gain + bias # The unfused way.  # noqa


def manual_bn(x, gain=None, bias=None, return_mean_var=False, eps=1e-5):
    """
    Manual BN
    Calculate means and variances using mean-of-squares minus mean-squared
    """

    # Cast x to float32 if necessary
    float_x = x.float()
    # Calculate expected value of x (m) and expected value of x**2 (m2)
    # Mean of x
    m = torch.mean(float_x, [0, 2, 3], keepdim=True)
    # Mean of x squared
    m2 = torch.mean(float_x**2, [0, 2, 3], keepdim=True)
    # Calculate variance as mean of squared minus mean squared.
    var = m2 - m**2
    # Cast back to float 16 if necessary
    var = var.type(x.type())
    m = m.type(x.type())
    # Return mean and variance for updating stored mean/var if requested
    if return_mean_var:
        return (
            fused_bn(x, m, var, gain, bias, eps),
            m.squeeze(),
            var.squeeze(),
        )
    else:
        return fused_bn(x, m, var, gain, bias, eps)


class myBN(nn.Module):
    """
    My batchnorm, supports standing stats
    """

    #: Constructor
    def __init__(self, num_channels, eps=1e-5, momentum=0.1):
        super(myBN, self).__init__()
        #: momentum for updating running stats
        self.momentum = momentum
        #: epsilon to avoid dividing by 0
        self.eps = eps
        # Momentum
        self.momentum = momentum
        # Register buffers
        self.register_buffer("stored_mean", torch.zeros(num_channels))
        self.register_buffer("stored_var", torch.ones(num_channels))
        self.register_buffer("accumulation_counter", torch.zeros(1))
        #: Accumulate running means and vars
        self.accumulate_standing = False

    #: reset standing stats
    def reset_stats(self):
        # pylint: disable=no-member
        self.stored_mean[:] = 0
        self.stored_var[:] = 0
        self.accumulation_counter[:] = 0

    #: forward
    def forward(self, x, gain, bias):
        # pylint: disable=no-member
        if self.training:
            out, mean, var = manual_bn(
                x, gain, bias, return_mean_var=True, eps=self.eps
            )
            # If accumulating standing stats, increment them
            if self.accumulate_standing:
                self.stored_mean[:] = self.stored_mean + mean.data
                self.stored_var[:] = self.stored_var + var.data
                self.accumulation_counter += 1.0
            # If not accumulating standing stats, take running averages
            else:
                self.stored_mean[:] = (
                    self.stored_mean * (1 - self.momentum) + mean * self.momentum
                )
                self.stored_var[:] = (
                    self.stored_var * (1 - self.momentum) + var * self.momentum
                )
            return out
        # If not in training mode, use the stored statistics
        else:
            mean = self.stored_mean.view(1, -1, 1, 1)
            var = self.stored_var.view(1, -1, 1, 1)
            # If using standing stats, divide them by the accumulation counter
            if self.accumulate_standing:
                mean = mean / self.accumulation_counter
                var = var / self.accumulation_counter
            return fused_bn(x, mean, var, gain, bias, self.eps)


class bn(nn.Module):
    """
    Normal, non-class-conditional BN
    """

    #: Constructor
    def __init__(
        self,
        output_size,
        eps=1e-5,
        momentum=0.1,
        cross_replica=False,
        mybn=False,
    ):
        super(bn, self).__init__()
        #: output size
        self.output_size = output_size
        #: Prepare gain and bias layers
        self.gain = P(torch.ones(output_size), requires_grad=True)
        #: bias
        self.bias = P(torch.zeros(output_size), requires_grad=True)
        #: epsilon to avoid dividing by 0
        self.eps = eps
        #: Momentum
        self.momentum = momentum
        #: Use cross-replica batchnorm?
        self.cross_replica = cross_replica
        #: Use my batchnorm?
        self.mybn = mybn

        if mybn:
            self.bn = myBN(output_size, self.eps, self.momentum)
        # Register buffers if neither of the above
        else:
            self.register_buffer("stored_mean", torch.zeros(output_size))
            self.register_buffer("stored_var", torch.ones(output_size))

    #: forward
    def forward(self, x):
        if self.mybn:
            gain = self.gain.view(1, -1, 1, 1)
            bias = self.bias.view(1, -1, 1, 1)
            return self.bn(x, gain=gain, bias=bias)
        else:
            return F.batch_norm(
                x,
                self.stored_mean,
                self.stored_var,
                self.gain,
                self.bias,
                self.training,
                self.momentum,
                self.eps,
            )


class ccbn(nn.Module):
    """
    Class-conditional bn
    output size is the number of channels, input size is for the linear layers
    Andy's Note: this class feels messy but I'm not really sure how to clean it up  # noqa
    Suggestions welcome! (By which I mean, refactor this and make a pull request
    if you want to make this more readable/usable).
    """

    #: Constructor
    def __init__(
        self,
        output_size,
        input_size,
        which_linear,
        eps=1e-5,
        momentum=0.1,
        cross_replica=False,
        mybn=False,
        norm_style="bn",
    ):
        super(ccbn, self).__init__()
        #: output size
        self.output_size, self.input_size = output_size, input_size
        #: Prepare gain and bias layers
        self.gain = which_linear(input_size, output_size)
        #: bias
        self.bias = which_linear(input_size, output_size)
        #: epsilon to avoid dividing by 0
        self.eps = eps
        #: Momentum
        self.momentum = momentum
        #: Use cross-replica batchnorm?
        self.cross_replica = cross_replica
        #: Use my batchnorm?
        self.mybn = mybn
        #: Norm style?
        self.norm_style = norm_style

        if self.mybn:
            #: bn
            self.bn = myBN(output_size, self.eps, self.momentum)
        elif self.norm_style in ["bn", "in"]:
            self.register_buffer("stored_mean", torch.zeros(output_size))
            self.register_buffer("stored_var", torch.ones(output_size))

    #: forward
    def forward(self, x, y):
        # Calculate class-conditional gains and biases
        gain = (1 + self.gain(y)).view(y.size(0), -1, 1, 1)
        bias = self.bias(y).view(y.size(0), -1, 1, 1)
        # If using my batchnorm
        if self.mybn:
            return self.bn(x, gain=gain, bias=bias)
        # else:
        else:
            if self.norm_style == "bn":
                out = F.batch_norm(
                    x,
                    self.stored_mean,
                    self.stored_var,
                    None,
                    None,
                    self.training,
                    0.1,
                    self.eps,
                )
            elif self.norm_style == "in":
                out = F.instance_norm(
                    x,
                    self.stored_mean,
                    self.stored_var,
                    None,
                    None,
                    self.training,
                    0.1,
                    self.eps,
                )
            elif self.norm_style == "gn":
                out = groupnorm(x, self.normstyle)
            elif self.norm_style == "nonorm":
                out = x
            return out * gain + bias

    #: extra_repr
    def extra_repr(self):
        s = "out: {output_size}, in: {input_size},"
        s += " cross_replica={cross_replica}"
        return s.format(**self.__dict__)


class ILA(nn.Module):
    """
    Image_Linear_Attention
    """

    #: Constructor
    def __init__(
        self,
        chan,
        chan_out=None,
        kernel_size=1,
        padding=0,
        stride=1,
        key_dim=32,
        value_dim=64,
        heads=8,
        norm_queries=True,
    ):
        super().__init__()
        #: chan
        self.chan = chan
        chan_out = chan if chan_out is None else chan_out

        #: key dimension
        self.key_dim = key_dim
        #: value dimension
        self.value_dim = value_dim
        #: heads
        self.heads = heads

        #: norm queries
        self.norm_queries = norm_queries

        conv_kwargs = {"padding": padding, "stride": stride}
        #: q
        self.to_q = nn.Conv2d(chan, key_dim * heads, kernel_size, **conv_kwargs)
        #: k
        self.to_k = nn.Conv2d(chan, key_dim * heads, kernel_size, **conv_kwargs)
        #: v
        self.to_v = nn.Conv2d(chan, value_dim * heads, kernel_size, **conv_kwargs)

        out_conv_kwargs = {"padding": padding}
        #: to out
        self.to_out = nn.Conv2d(
            value_dim * heads, chan_out, kernel_size, **out_conv_kwargs
        )

    #: forward
    def forward(self, x, context=None):
        b, c, h, w, k_dim, heads = *x.shape, self.key_dim, self.heads

        q, k, v = (self.to_q(x), self.to_k(x), self.to_v(x))

        q, k, v = map(lambda t: t.reshape(b, heads, -1, h * w), (q, k, v))

        q, k = map(lambda x: x * (self.key_dim**-0.25), (q, k))

        if context is not None:
            context = context.reshape(b, c, 1, -1)
            ck, cv = self.to_k(context), self.to_v(context)
            ck, cv = map(lambda t: t.reshape(b, heads, k_dim, -1), (ck, cv))
            k = torch.cat((k, ck), dim=3)
            v = torch.cat((v, cv), dim=3)

        k = k.softmax(dim=-1)

        if self.norm_queries:
            q = q.softmax(dim=-2)

        context = torch.einsum("bhdn,bhen->bhde", k, v)
        out = torch.einsum("bhdn,bhde->bhen", q, context)
        out = out.reshape(b, -1, h, w)
        out = self.to_out(out)
        return out


class CBAM_attention(nn.Module):
    """CBAM attention"""

    #: Constructor
    def __init__(
        self,
        channels,
        which_conv=SNConv2d,
        reduction=8,
        attention_kernel_size=3,
    ):
        super(CBAM_attention, self).__init__()
        #: average pooling
        self.avg_pool = nn.AdaptiveAvgPool2d(1)
        #: max pooling
        self.max_pool = nn.AdaptiveMaxPool2d(1)
        #: fcl
        self.fc1 = which_conv(
            channels, channels // reduction, kernel_size=1, padding=0
        )
        #: relu
        self.relu = nn.ReLU(inplace=True)
        #: f2c
        self.fc2 = which_conv(
            channels // reduction, channels, kernel_size=1, padding=0
        )
        #: sigmoid channel
        self.sigmoid_channel = nn.Sigmoid()
        #: convolution after concatenation
        self.conv_after_concat = which_conv(
            2,
            1,
            kernel_size=attention_kernel_size,
            stride=1,
            padding=attention_kernel_size // 2,
        )
        #: sigmoid_spatial
        self.sigmoid_spatial = nn.Sigmoid()

    #: forward
    def forward(self, x):
        # Channel attention module
        module_input = x
        avg = self.avg_pool(x)
        mx = self.max_pool(x)
        avg = self.fc1(avg)
        mx = self.fc1(mx)
        avg = self.relu(avg)
        mx = self.relu(mx)
        avg = self.fc2(avg)
        mx = self.fc2(mx)
        x = avg + mx
        x = self.sigmoid_channel(x)
        # Spatial attention module
        x = module_input * x
        module_input = x
        # b, c, h, w = x.size()
        avg = torch.mean(x, 1, True)
        mx, _ = torch.max(x, 1, True)
        x = torch.cat((avg, mx), 1)
        x = self.conv_after_concat(x)
        x = self.sigmoid_spatial(x)
        x = module_input * x
        return x


class Attention(nn.Module):
    """Attention"""

    #: Constructor
    def __init__(self, ch, which_conv=SNConv2d):
        super(Attention, self).__init__()
        #: Channel multiplier
        self.ch = ch
        #: which_conv
        self.which_conv = which_conv
        #: theta
        self.theta = self.which_conv(
            self.ch, self.ch // 8, kernel_size=1, padding=0, bias=False
        )
        #: phi
        self.phi = self.which_conv(
            self.ch, self.ch // 8, kernel_size=1, padding=0, bias=False
        )
        #: g
        self.g = self.which_conv(
            self.ch, self.ch // 2, kernel_size=1, padding=0, bias=False
        )
        #: o
        self.o = self.which_conv(
            self.ch // 2, self.ch, kernel_size=1, padding=0, bias=False
        )
        #: Learnable gain parameter
        self.gamma = P(torch.tensor(0.0), requires_grad=True)

    #: forward
    def forward(self, x):
        # Apply convs
        theta = self.theta(x)
        phi = F.max_pool2d(self.phi(x), [2, 2])
        g = F.max_pool2d(self.g(x), [2, 2])
        # Perform reshapes
        theta = theta.view(-1, self.ch // 8, x.shape[2] * x.shape[3])
        phi = phi.view(-1, self.ch // 8, x.shape[2] * x.shape[3] // 4)
        g = g.view(-1, self.ch // 2, x.shape[2] * x.shape[3] // 4)
        # Matmul and softmax to get attention maps
        beta = F.softmax(torch.bmm(theta.transpose(1, 2), phi), -1)
        # Attention map times g path
        o = self.o(
            torch.bmm(g, beta.transpose(1, 2)).view(
                -1, self.ch // 2, x.shape[2], x.shape[3]
            )
        )
        return self.gamma * o + x


class SNEmbedding(nn.Embedding, SN):
    """
    Embedding layer with spectral norm
    We use num_embeddings as the dim instead of embedding_dim here
    for convenience sake
    """

    #: Constructor
    def __init__(
        self,
        num_embeddings,
        embedding_dim,
        padding_idx=None,
        max_norm=None,
        norm_type=2,
        scale_grad_by_freq=False,
        sparse=False,
        _weight=None,
        num_svs=1,
        num_itrs=1,
        eps=1e-12,
    ):
        nn.Embedding.__init__(
            self,
            num_embeddings,
            embedding_dim,
            padding_idx,
            max_norm,
            norm_type,
            scale_grad_by_freq,
            sparse,
            _weight,
        )
        SN.__init__(self, num_svs, num_itrs, num_embeddings, eps=eps)

    #: forward
    def forward(self, x):
        return F.embedding(x, self.W_())


def scaled_dot_product(q, k, v):
    d_k = q.size()[-1]
    attn_logits = torch.matmul(q, k.transpose(-2, -1))
    attn_logits = attn_logits / math.sqrt(d_k)
    attention = F.softmax(attn_logits, dim=-1)
    values = torch.matmul(attention, v)
    return values, attention


class MultiheadAttention(nn.Module):
    """MultiheadAttention"""

    #: Constructor
    def __init__(self, input_dim, embed_dim, num_heads, which_linear):
        super().__init__()
        assert (
            embed_dim % num_heads == 0
        ), "Embedding dimension must be 0 modulo number of heads."

        #: embedding dimension
        self.embed_dim = embed_dim
        #: number of heads
        self.num_heads = num_heads
        #: head dimension
        self.head_dim = embed_dim // num_heads
        #: which linear
        self.which_linear = which_linear

        # Stack all weight matrices 1...h together for efficiency
        #: qkv projection
        self.qkv_proj = self.which_linear(input_dim, 3 * embed_dim)
        #: o projection
        self.o_proj = self.which_linear(embed_dim, embed_dim)

        self._reset_parameters()

    #: reset parameters
    def _reset_parameters(self):
        # Original Transformer initialization, see PyTorch documentation
        nn.init.xavier_uniform_(self.qkv_proj.weight)
        self.qkv_proj.bias.data.fill_(0)
        nn.init.xavier_uniform_(self.o_proj.weight)
        self.o_proj.bias.data.fill_(0)

    #: forward
    def forward(self, x, return_attention=False):
        batch_size, seq_length, embed_dim = x.size()
        qkv = self.qkv_proj(x)

        # Separate Q, K, V from linear output
        qkv = qkv.reshape(batch_size, seq_length, self.num_heads, 3 * self.head_dim)
        qkv = qkv.permute(0, 2, 1, 3)  # [Batch, Head, SeqLen, Dims]
        q, k, v = qkv.chunk(3, dim=-1)

        # Determine value outputs
        values, attention = scaled_dot_product(q, k, v)
        values = values.permute(0, 2, 1, 3)  # [Batch, SeqLen, Head, Dims]
        values = values.reshape(batch_size, seq_length, embed_dim)
        o = self.o_proj(values)

        if return_attention:
            return o, attention
        else:
            return o


class EncoderBlock(nn.Module):
    """EncoderBlock"""

    #: Constructor
    def __init__(self, input_dim, num_heads, dim_feedforward, dropout, which_linear):
        """
        Inputs:
            input_dim - Dimensionality of the input
            num_heads - Number of heads to use in the attention block
            dim_feedforward - Dimensionality of the hidden layer in the MLP
            dropout - Dropout probability to use in the dropout layers
        """
        super().__init__()

        #: which linear
        self.which_linear = which_linear
        #: Attention layer
        self.self_attn = MultiheadAttention(
            input_dim, input_dim, num_heads, which_linear
        )

        #: Two-layer MLP
        self.linear_net = nn.Sequential(
            self.which_linear(input_dim, dim_feedforward),
            nn.Dropout(dropout),
            nn.ReLU(inplace=True),
            self.which_linear(dim_feedforward, input_dim),
        )

        # Layers to apply in between the main layers
        #: norm1
        self.norm1 = nn.LayerNorm(input_dim)
        #: norm2
        self.norm2 = nn.LayerNorm(input_dim)
        #: dropout
        self.dropout = nn.Dropout(dropout)

    #: forward
    def forward(self, x):
        # Attention part
        x_pre1 = self.norm1(x)
        attn_out = self.self_attn(x_pre1)
        x = x + self.dropout(attn_out)
        # x = self.norm1(x)

        # MLP part
        x_pre2 = self.norm2(x)
        linear_out = self.linear_net(x_pre2)
        x = x + self.dropout(linear_out)
        # x = self.norm2(x)

        return x


class RelationalReasoning(nn.Module):
    """RelationalReasoning"""

    #: Constructor
    def __init__(self, num_layers, hidden_dim, **block_args):
        super().__init__()
        #: layers
        self.layers = nn.ModuleList(
            [EncoderBlock(**block_args) for _ in range(num_layers)]
        )
        #: normalization
        self.norm = nn.LayerNorm(hidden_dim)

    #: forward
    def forward(self, x):
        for layer in self.layers:
            x = layer(x)

        x = self.norm(x)
        return x

    #: get attention maps
    def get_attention_maps(self, x):
        attention_maps = []
        for layer in self.layers:
            _, attn_map = layer.self_attn(x, return_attention=True)
            attention_maps.append(attn_map)
            x = layer(x)
        return attention_maps


class GBlock(nn.Module):
    """GBlock"""

    #: Constructor
    def __init__(
        self,
        in_channels,
        out_channels,
        which_conv=SNConv2d,
        which_bn=bn,
        activation=None,
        upsample=None,
        channel_ratio=4,
    ):
        super(GBlock, self).__init__()

        #: input channels
        self.in_channels, self.out_channels = in_channels, out_channels
        #: hidden channels
        self.hidden_channels = self.in_channels // channel_ratio
        #: which convolution
        self.which_conv, self.which_bn = which_conv, which_bn
        #: activation
        self.activation = activation
        # Conv layers
        #: conv1
        self.conv1 = self.which_conv(
            self.in_channels, self.hidden_channels, kernel_size=1, padding=0
        )
        #: conv2
        self.conv2 = self.which_conv(self.hidden_channels, self.hidden_channels)
        #: conv3
        self.conv3 = self.which_conv(self.hidden_channels, self.hidden_channels)
        #: conv4
        self.conv4 = self.which_conv(
            self.hidden_channels, self.out_channels, kernel_size=1, padding=0
        )
        # Batchnorm layers
        #: bn1
        self.bn1 = self.which_bn(self.in_channels)
        #: bn2
        self.bn2 = self.which_bn(self.hidden_channels)
        #: bn3
        self.bn3 = self.which_bn(self.hidden_channels)
        #: bn4
        self.bn4 = self.which_bn(self.hidden_channels)
        #: upsample layers
        self.upsample = upsample

    #: forward
    def forward(self, x, y):
        # Project down to channel ratio
        h = self.conv1(self.activation(self.bn1(x, y)))
        # Apply next BN-ReLU
        h = self.activation(self.bn2(h, y))
        # Drop channels in x if necessary
        if self.in_channels != self.out_channels:
            x = x[:, : self.out_channels]
        # Upsample both h and x at this point
        if self.upsample:
            h = self.upsample(h)
            x = self.upsample(x)
        # 3x3 convs
        h = self.conv2(h)
        h = self.conv3(self.activation(self.bn3(h, y)))
        # Final 1x1 conv
        h = self.conv4(self.activation(self.bn4(h, y)))
        return h + x


def G_arch(ch=64, attention="64"):
    arch = {}
    arch[512] = {
        "in_channels": [ch * item for item in [16, 16, 8, 8, 4, 2, 1]],
        "out_channels": [ch * item for item in [16, 8, 8, 4, 2, 1, 1]],
        "upsample": [True] * 7,
        "resolution": [8, 16, 32, 64, 128, 256, 512],
        "attention": {
            2**i: (2 ** i in [int(item) for item in attention.split("_")])
            for i in range(3, 10)
        },
    }
    arch[256] = {
        "in_channels": [ch * item for item in [16, 16, 8, 8, 4, 2]],
        "out_channels": [ch * item for item in [16, 8, 8, 4, 2, 1]],
        "upsample": [True] * 6,
        "resolution": [8, 16, 32, 64, 128, 256],
        "attention": {
            2**i: (2 ** i in [int(item) for item in attention.split("_")])
            for i in range(3, 9)
        },
    }
    arch[128] = {
        "in_channels": [ch * item for item in [16, 16, 8, 4, 2]],
        "out_channels": [ch * item for item in [16, 8, 4, 2, 1]],
        "upsample": [True] * 5,
        "resolution": [8, 16, 32, 64, 128],
        "attention": {
            2**i: (2 ** i in [int(item) for item in attention.split("_")])
            for i in range(3, 8)
        },
    }
    arch[96] = {
        "in_channels": [ch * item for item in [16, 16, 8, 4]],
        "out_channels": [ch * item for item in [16, 8, 4, 2]],
        "upsample": [True] * 4,
        "resolution": [12, 24, 48, 96],
        "attention": {
            12 * 2**i: (6 * 2 ** i in [int(item) for item in attention.split("_")])
            for i in range(0, 4)
        },
    }

    arch[64] = {
        "in_channels": [ch * item for item in [16, 16, 8, 4]],
        "out_channels": [ch * item for item in [16, 8, 4, 2]],
        "upsample": [True] * 4,
        "resolution": [8, 16, 32, 64],
        "attention": {
            2**i: (2 ** i in [int(item) for item in attention.split("_")])
            for i in range(3, 7)
        },
    }
    arch[32] = {
        "in_channels": [ch * item for item in [4, 4, 4]],
        "out_channels": [ch * item for item in [4, 4, 4]],
        "upsample": [True] * 3,
        "resolution": [8, 16, 32],
        "attention": {
            2**i: (2 ** i in [int(item) for item in attention.split("_")])
            for i in range(3, 6)
        },
    }

    return arch


class Generator(nn.Module):
    """Generator"""

    #: Constructor
    def __init__(
        self,
        G_ch=64,
        G_depth=2,
        dim_z=128,
        bottom_width=4,
        resolution=256,
        G_kernel_size=3,
        G_attn="64",
        n_classes=40,
        H_base=1,
        num_G_SVs=1,
        num_G_SV_itrs=1,
        attn_type="sa",
        G_shared=True,
        shared_dim=128,
        hier=True,
        cross_replica=False,
        mybn=False,
        G_activation=nn.ReLU(inplace=False),
        G_lr=5e-5,
        G_B1=0.0,
        G_B2=0.999,
        adam_eps=1e-8,
        BN_eps=1e-5,
        SN_eps=1e-12,
        G_init="ortho",
        G_mixed_precision=False,
        G_fp16=False,
        skip_init=False,
        no_optim=False,
        sched_version="default",
        RRM_prx_G=True,
        n_head_G=2,
        G_param="SN",
        norm_style="bn",
        **kwargs
    ):
        super(Generator, self).__init__()
        #: Channel width mulitplier
        self.ch = G_ch
        #: Number of resblocks per stage
        self.G_depth = G_depth
        #: Dimensionality of the latent space
        self.dim_z = dim_z
        #: The initial spatial dimensions
        self.bottom_width = bottom_width
        #: The initial harizontal dimension
        self.H_base = H_base
        #: Resolution of the output
        self.resolution = resolution
        #: Kernel size?
        self.kernel_size = G_kernel_size
        #: Attention?
        self.attention = G_attn
        #: number of classes, for use in categorical conditional generation
        self.n_classes = n_classes
        #: Use shared embeddings?
        self.G_shared = G_shared
        #: Dimensionality of the shared embedding? Unused if not using G_shared
        self.shared_dim = shared_dim if shared_dim > 0 else dim_z
        #: Hierarchical latent space?
        self.hier = hier
        #: Cross replica batchnorm?
        self.cross_replica = cross_replica
        #: Use my batchnorm?
        self.mybn = mybn
        # nonlinearity for residual blocks
        if G_activation == "inplace_relu":
            #: activation
            self.activation = torch.nn.ReLU(inplace=True)
        elif G_activation == "relu":
            self.activation = torch.nn.ReLU(inplace=False)
        elif G_activation == "leaky_relu":
            self.activation = torch.nn.LeakyReLU(0.2, inplace=False)
        else:
            raise NotImplementedError("activation function not implemented")
        #: Initialization style
        self.init = G_init
        #: Parameterization style
        self.G_param = G_param
        #: Normalization style
        self.norm_style = norm_style
        #: Epsilon for BatchNorm?
        self.BN_eps = BN_eps
        #: Epsilon for Spectral Norm?
        self.SN_eps = SN_eps
        #: fp16?
        self.fp16 = G_fp16
        #: Architecture dict
        self.arch = G_arch(self.ch, self.attention)[resolution]
        #: RRM_prx_G
        self.RRM_prx_G = RRM_prx_G
        #: n_head_G
        self.n_head_G = n_head_G

        # Which convs, batchnorms, and linear layers to use
        if self.G_param == "SN":
            #: which conv
            self.which_conv = functools.partial(
                SNConv2d,
                kernel_size=3,
                padding=1,
                num_svs=num_G_SVs,
                num_itrs=num_G_SV_itrs,
                eps=self.SN_eps,
            )
            #: which linear
            self.which_linear = functools.partial(
                SNLinear,
                num_svs=num_G_SVs,
                num_itrs=num_G_SV_itrs,
                eps=self.SN_eps,
            )
        else:
            self.which_conv = functools.partial(nn.Conv2d, kernel_size=3, padding=1)
            self.which_linear = nn.Linear

        # We use a non-spectral-normed embedding here regardless;
        # For some reason applying SN to G's embedding seems to randomly cripple G  # noqa
        #: which embedding
        self.which_embedding = nn.Embedding
        bn_linear = (
            functools.partial(self.which_linear, bias=False)
            if self.G_shared
            else self.which_embedding
        )
        #: which bn
        self.which_bn = functools.partial(
            ccbn,
            which_linear=bn_linear,
            cross_replica=self.cross_replica,
            mybn=self.mybn,
            input_size=(
                self.shared_dim + self.dim_z if self.G_shared else self.n_classes
            ),
            norm_style=self.norm_style,
            eps=self.BN_eps,
        )
        #: shared
        self.shared = (
            self.which_embedding(n_classes, self.shared_dim)
            if G_shared
            else identity()
        )

        if self.RRM_prx_G:
            #: RRM on proxy embeddings
            self.RR_G = RelationalReasoning(
                num_layers=1,
                input_dim=128,
                dim_feedforward=128,
                which_linear=nn.Linear,
                num_heads=self.n_head_G,
                dropout=0.0,
                hidden_dim=128,
            )

        #: First linear layer
        self.linear = self.which_linear(
            self.dim_z + self.shared_dim,
            self.arch["in_channels"][0] * ((self.bottom_width**2) * self.H_base),
        )

        # self.blocks is a doubly-nested list of modules, the outer loop intended  # noqa
        # to be over blocks at a given resolution (resblocks and/or self-attention)  # noqa
        # while the inner loop is over a given block
        #: blocks
        self.blocks = []
        for index in range(len(self.arch["out_channels"])):
            self.blocks += [
                [
                    GBlock(
                        in_channels=self.arch["in_channels"][index],
                        out_channels=self.arch["in_channels"][index]
                        if g_index == 0
                        else self.arch["out_channels"][index],
                        which_conv=self.which_conv,
                        which_bn=self.which_bn,
                        activation=self.activation,
                        upsample=(
                            functools.partial(F.interpolate, scale_factor=2)
                            if self.arch["upsample"][index]
                            and g_index == (self.G_depth - 1)
                            else None
                        ),
                    )
                ]
                for g_index in range(self.G_depth)
            ]

            # If attention on this block, attach it to the end
            if self.arch["attention"][self.arch["resolution"][index]]:
                print(
                    f"Adding attention layer in G at resolution {self.arch['resolution'][index]:d}"
                )

                if attn_type == "sa":
                    self.blocks[-1] += [
                        Attention(self.arch["out_channels"][index], self.which_conv)
                    ]
                elif attn_type == "cbam":
                    self.blocks[-1] += [
                        CBAM_attention(
                            self.arch["out_channels"][index], self.which_conv
                        )
                    ]
                elif attn_type == "ila":
                    self.blocks[-1] += [ILA(self.arch["out_channels"][index])]

        # Turn self.blocks into a ModuleList so that it's all properly registered.  # noqa
        self.blocks = nn.ModuleList([nn.ModuleList(block) for block in self.blocks])

        # output layer: batchnorm-relu-conv.
        # Consider using a non-spectral conv here
        #: output layer
        self.output_layer = nn.Sequential(
            bn(
                self.arch["out_channels"][-1],
                cross_replica=self.cross_replica,
                mybn=self.mybn,
            ),
            self.activation,
            self.which_conv(self.arch["out_channels"][-1], 1),
        )

        # Initialize weights. Optionally skip init for testing.
        if not skip_init:
            self.init_weights()

        # Set up optimizer
        # If this is an EMA copy, no need for an optim, so just return now
        if no_optim:
            return
        #: lr
        self.lr = G_lr
        #: B1
        self.B1 = G_B1
        #: B2
        self.B2 = G_B2
        #: adam_eps
        self.adam_eps = adam_eps
        if G_mixed_precision:
            print("Using fp16 adam in G...")
            import utils

            self.optim = utils.Adam16(
                params=self.parameters(),
                lr=self.lr,
                betas=(self.B1, self.B2),
                weight_decay=0,
                eps=self.adam_eps,
            )

        #: optim
        self.optim = optim.Adam(
            params=self.parameters(),
            lr=self.lr,
            betas=(self.B1, self.B2),
            weight_decay=0,
            eps=self.adam_eps,
        )
        # LR scheduling
        if sched_version == "default":
            #:  lr sched
            self.lr_sched = None
        elif sched_version == "CosAnnealLR":
            self.lr_sched = optim.lr_scheduler.CosineAnnealingLR(
                self.optim,
                T_max=kwargs["num_epochs"],
                eta_min=self.lr / 4,
                last_epoch=-1,
            )
        elif sched_version == "CosAnnealWarmRes":
            self.lr_sched = optim.lr_scheduler.CosineAnnealingWarmRestarts(
                self.optim, T_0=10, T_mult=2, eta_min=self.lr / 4
            )
        else:
            self.lr_sched = None

    #: Initialize
    def init_weights(self):
        #: parameter count
        self.param_count = 0
        for module in self.modules():
            if (
                isinstance(module, nn.Conv2d)
                or isinstance(module, nn.Linear)
                or isinstance(module, nn.Embedding)
            ):
                if self.init == "ortho":
                    init.orthogonal_(module.weight)
                elif self.init == "N02":
                    init.normal_(module.weight, 0, 0.02)
                elif self.init in ["glorot", "xavier"]:
                    init.xavier_uniform_(module.weight)
                else:
                    print("Init style not recognized...")
                self.param_count += sum(
                    [p.data.nelement() for p in module.parameters()]
                )
        print(f"Param count for G's initialized parameters: {self.param_count}")

    #: forward
    def forward(self, z, y):
        y = self.shared(y)
        # If relational embedding
        if self.RRM_prx_G:
            y = self.RR_G(y.unsqueeze(0)).squeeze(0)
            # y = F.normalize(y, dim=1)
        # If hierarchical, concatenate zs and ys
        if self.hier:  # y and z are [bs,128] dimensional
            z = torch.cat([y, z], 1)
            y = z
        # First linear layer
        h = self.linear(z)  # ([bs,256]-->[bs,24576])
        # Reshape
        h = h.view(h.size(0), -1, self.bottom_width, self.bottom_width * self.H_base)
        # Loop over blocks
        for _, blocklist in enumerate(self.blocks):
            # Second inner loop in case block has multiple layers
            for block in blocklist:
                h = block(h, y)

        # Apply batchnorm-relu-conv-tanh at output
        return torch.tanh(self.output_layer(h))


class Model(Generator):
    """
    Generator subclass
    default initializing with CONFIG dict
    """

    #: Constructor
    def __init__(self):
        super().__init__(**CONFIG)


def generate(model: nn.Module):
    """
    Run inference with the provided Generator model

    Args:
        model (nn.Module): Generator model

    Returns:
        torch.Tensor: batch of 40 PXD images
    """
    device = next(model.parameters()).device
    with torch.no_grad():
        latents = torch.randn(40, 128, device=device)
        labels = torch.tensor(list(range(40)), dtype=torch.long, device=device)
        imgs = model(latents, labels).detach().cpu()
        # Cut the noise below 7 ADU
        imgs = F.threshold(imgs, -0.26, -1)
        # center range [-1, 1] to [0, 1]
        imgs = imgs.mul_(0.5).add_(0.5)
        # renormalize and convert to uint8
        imgs = torch.pow(256, imgs).add_(-1).clamp_(0, 255).to(torch.uint8)
        # flatten channel dimension and crop 256 to 250
        imgs = imgs[:, 0, 3:-3, :]
        return imgs
