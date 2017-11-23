samples = {
    "Bu_Rho0Kst+_K+pi0_Transverse":
        {"mode_name": "Bu_Rho0Kst+_K+pi0_Transverse",
         "tex_name": "$B^+\\to\\rho^0K^{*+}(\\to K^+\\pi^0)$",
         "polarisation": "with transverse polarisation ",
         "final_state": "$K\\pi^0$"},


    "Bu_Rho0Kst+_K+pi0_Longitudinal":
        {"mode_name": "Bu_Rho0Kst+_K+pi0_Longitudinal",
         "tex_name": "$B^+\\to\\rho^0K^{*+}(\\to K^+\\pi^0)$",
         "polarisation": "with longitudinal polarisation ",
         "final_state": "$K\\pi^0$"},
}


def full_sample_name(mode_name):
    if mode_name not in samples.keys():
        return mode_name.replace("_", "\\_")
    else:
        return samples[mode_name]["tex_name"] + "\\ " + samples[mode_name]["polarisation"] + \
            " reconstructed with " + samples[mode_name]["final_state"] + "\\ in final state"
