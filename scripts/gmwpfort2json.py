from sro import gwnd
from sro import gmwpfort

import json
import dataclasses

settingsfolder = "../BinOut/RelWithDebInfo/setting/"

waypoints = gmwpfort.read(settingsfolder + "gmwpfort.dat")

with open(settingsfolder + "gmwpfort.json", "w") as f:
    f.write(json.dumps(waypoints, cls=gwnd.EnhancedJSONEncoder))
