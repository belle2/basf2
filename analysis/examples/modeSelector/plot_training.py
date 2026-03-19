#!/usr/bin/env python3
"""
Plot training diagnostics from a ModeSelector checkpoint.

Usage:
    python3 plot_training.py networks/net_category.pt
    python3 plot_training.py networks/net_category.pt networks/net_main.pt
    python3 plot_training.py networks/net_category.pt --output plots/
"""

import argparse
import os

import matplotlib.pyplot as plt
import numpy as np
import torch


def load_checkpoint(path):
    ckpt = torch.load(path, map_location='cpu')
    name = os.path.splitext(os.path.basename(path))[0]
    return ckpt, name


def plot_loss_curves(ckpt, name, ax_loss, ax_lr):
    """Plot train/val loss and learning rate per epoch."""
    history = ckpt.get('history')

    if history is None:
        # Show only the best-epoch point when per-epoch history is unavailable.
        best_epoch = ckpt['epoch']
        ax_loss.scatter([best_epoch + 1], [ckpt['train_loss']], marker='o',
                        label='train (best epoch only)')
        ax_loss.scatter([best_epoch + 1], [ckpt['val_loss']], marker='s',
                        label='val (best epoch only)')
        ax_loss.set_title(f'{name} -- no loss history in checkpoint')
        ax_lr.set_visible(False)
        return

    epochs = range(1, len(history['train_loss']) + 1)
    best_epoch = ckpt['epoch'] + 1  # 0-indexed in checkpoint

    ax_loss.plot(epochs, history['train_loss'], label='train')
    ax_loss.plot(epochs, history['val_loss'], label='val')
    if any(x > 0 for x in history['disco_loss']):
        ax_loss.plot(epochs, history['disco_loss'], label='disco', linestyle='--', alpha=0.7)
    ax_loss.axvline(best_epoch, color='gray', linestyle=':', alpha=0.8, label=f'best (ep {best_epoch})')
    ax_loss.set_xlabel('epoch')
    ax_loss.set_ylabel('loss')
    ax_loss.set_title(f'{name} -- loss curves')
    ax_loss.legend()
    ax_loss.grid(True, alpha=0.3)

    ax_lr.plot(epochs, history['lr'])
    ax_lr.set_xlabel('epoch')
    ax_lr.set_ylabel('learning rate')
    ax_lr.set_title('learning rate schedule')
    ax_lr.set_yscale('log')
    ax_lr.grid(True, alpha=0.3)


def plot_weight_distributions(ckpt, name, fig):
    """Histogram of weights per layer."""
    sd = ckpt['model_state_dict']
    weight_layers = [(k, v.numpy().ravel()) for k, v in sd.items() if 'weight' in k]

    n = len(weight_layers)
    axes = fig.subplots(1, n)
    if n == 1:
        axes = [axes]

    for ax, (layer_name, weights) in zip(axes, weight_layers):
        ax.hist(weights, bins=60, density=True)
        ax.set_title(layer_name.replace('network.', 'layer '), fontsize=8)
        ax.set_xlabel('weight value', fontsize=7)
        ax.tick_params(labelsize=7)
        std = weights.std()
        ax.axvline(0, color='k', linewidth=0.5)
        ax.text(0.97, 0.97, f'std={std:.3f}', transform=ax.transAxes,
                ha='right', va='top', fontsize=7)

    fig.suptitle(f'{name} -- weight distributions')


def plot_input_importance(ckpt, name, ax):
    """L2 norm of first-layer weights per input feature (top features highlighted)."""
    sd = ckpt['model_state_dict']
    # First layer weight: shape (256, n_inputs)
    first_weight = None
    for k, v in sd.items():
        if 'weight' in k:
            first_weight = v.numpy()
            break

    if first_weight is None:
        ax.set_visible(False)
        return

    importance = np.linalg.norm(first_weight, axis=0)  # L2 norm over output units
    has_inputs = ckpt.get('has_inputs', list(range(len(importance))))

    ax.bar(range(len(importance)), importance, width=1.0, linewidth=0)
    ax.set_xlabel('feature index (within selected features)')
    ax.set_ylabel('L2 norm of input weights')
    ax.set_title(f'{name} -- input feature importance (first layer)')
    ax.grid(True, axis='y', alpha=0.3)

    # Annotate top 10
    top10 = np.argsort(importance)[-10:][::-1]
    for rank, idx in enumerate(top10):
        global_idx = has_inputs[idx] if idx < len(has_inputs) else idx
        ax.annotate(f'{global_idx}', xy=(idx, importance[idx]),
                    xytext=(0, 3), textcoords='offset points',
                    ha='center', fontsize=6, rotation=90)


def summarise(ckpt, name):
    sd = ckpt['model_state_dict']
    n_params = sum(v.numel() for v in sd.values())
    cfg = ckpt.get('config', {})
    print(f"\n{'='*50}")
    print(f"Checkpoint: {name}")
    print(f"  Best epoch:    {ckpt['epoch'] + 1}")
    print(f"  Best val loss: {ckpt['val_loss']:.6f}")
    print(f"  Train loss:    {ckpt['train_loss']:.6f}")
    print(f"  Parameters:    {n_params:,}")
    print(f"  Input size:    {cfg.get('input_size', '?')}")
    print(f"  Num labels:    {cfg.get('num_labels', '?')}")
    print(f"  Network type:  {cfg.get('network_type', '?')}")
    print(f"  has_inputs:    {len(ckpt.get('has_inputs', []))} features")
    print(f"  History:       {'yes' if 'history' in ckpt else 'no'}")


def main():
    parser = argparse.ArgumentParser(description='Plot ModeSelector training diagnostics')
    parser.add_argument('checkpoints', nargs='+', help='Path(s) to .pt checkpoint file(s)')
    parser.add_argument('--output', default=None,
                        help='Output directory for plots (default: show interactively)')
    args = parser.parse_args()

    if args.output:
        os.makedirs(args.output, exist_ok=True)

    for path in args.checkpoints:
        ckpt, name = load_checkpoint(path)
        summarise(ckpt, name)

        # --- Figure 1: loss curves + LR ---
        fig1, (ax_loss, ax_lr) = plt.subplots(1, 2, figsize=(12, 4))
        fig1.suptitle(name)
        plot_loss_curves(ckpt, name, ax_loss, ax_lr)
        fig1.tight_layout()
        if args.output:
            out = os.path.join(args.output, f'{name}_loss.pdf')
            fig1.savefig(out, bbox_inches='tight')
            print(f"Saved {out}")

        # --- Figure 2: weight distributions ---
        sd = ckpt['model_state_dict']
        n_layers = sum(1 for k in sd if 'weight' in k)
        fig2 = plt.figure(figsize=(3 * n_layers, 3))
        plot_weight_distributions(ckpt, name, fig2)
        fig2.tight_layout()
        if args.output:
            out = os.path.join(args.output, f'{name}_weights.pdf')
            fig2.savefig(out, bbox_inches='tight')
            print(f"Saved {out}")

        # --- Figure 3: input importance ---
        fig3, ax3 = plt.subplots(figsize=(14, 4))
        plot_input_importance(ckpt, name, ax3)
        fig3.tight_layout()
        if args.output:
            out = os.path.join(args.output, f'{name}_importance.pdf')
            fig3.savefig(out, bbox_inches='tight')
            print(f"Saved {out}")

    if not args.output:
        plt.show()
    else:
        plt.close('all')


if __name__ == '__main__':
    main()
