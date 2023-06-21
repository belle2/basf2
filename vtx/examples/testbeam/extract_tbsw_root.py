import ROOT
import pandas as pd


selected_keys = {
    "hres_u_all": 'RMS',
    "hfit_sigma_u": 'Mean',
    "hpoint_resolution_u": 'Bin',
    "hres_v_all": 'RMS',
    "hfit_sigma_v": 'Mean',
    "hpoint_resolution_v": 'Bin',
    "hCharge": 'Mean',
    "hCCharge": 'Mean',
    "hsize": 'Mean',
    "hsizeU": 'Mean',
    "hsizeV": 'Mean',
}


y_labels = {
    "hres_u_all": 'RMS u residuals',
    "hfit_sigma_u": 'Mean telscope $\\sigma_u$ at DUT',
    "hpoint_resolution_u": 'Mean DUT $\\sigma_u$',
    "hres_v_all": 'RMS v residuals',
    "hfit_sigma_v": 'Mean telescope $\\sigma_v$ at DUT',
    "hpoint_resolution_v": 'Mean DUT $\\sigma_v$',
    "hCharge": 'Mean seed ToT',
    "hCCharge": 'Mean cluster charge',
    "hsize": 'Mean cluster size',
    "hsizeU": 'Mean columns per cluster',
    "hsizeV": 'Mean rows per cluster',
}

y_units = {
    "hres_u_all": '$\\mu$m',
    "hfit_sigma_u": '$\\mu$m',
    "hpoint_resolution_u": '$\\mu$m',
    "hres_v_all": '$\\mu$m',
    "hfit_sigma_v": '$\\mu$m',
    "hpoint_resolution_v": '$\\mu$m',
    "hCharge": '25ns',
    "hCCharge": '25ns',
    "hsize": 'pixels',
    "hsizeU": 'columns',
    "hsizeV": 'rows',
}


def get_matches(root_files_mc, x_axis_mc, root_files_data, x_axis_data):

    matches = []

    for i, x in enumerate(x_axis_mc):
        if x in x_axis_data:
            matches.append((x, root_files_mc[i], root_files_data[x_axis_data.index(x)]))

    return matches


def get_object(root_files, angles):

    columns = [x.split('/')[-1] for x in selected_keys.keys()]
    df = pd.DataFrame(columns=columns, index=angles)

    for i, file in enumerate(root_files):
        try:
            f = ROOT.TFile(file)
        except Exception:
            continue
        list_per_file = []
        for key in selected_keys.keys():
            plot = f.Get(key)
            # print(key)
            if selected_keys[key] == 'RMS' or selected_keys[key] == 'Mean':
                attr = getattr(plot, "Get%s" % selected_keys[key])
                # error = getattr(plot, "Get%sError" % selected_keys[key])
                val = attr()
                list_per_file.append(val)
                # print(val)
            if selected_keys[key] == 'Bin':
                attr = getattr(plot, "GetBinContent")
                val = attr(1)
                list_per_file.append(val)
                # print(val)

        df.loc[angles[i]] = list_per_file
    return df
