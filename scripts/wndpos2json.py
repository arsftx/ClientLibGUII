from sro import gwnd
from sro import wndpos

import json
import dataclasses

settingsfolder = "../BinOut/RelWithDebInfo/setting/"

windowspos = wndpos.read(settingsfolder + "wndpos.dat")

with open(settingsfolder + "wndpos.json", "w") as f:
    f.write(json.dumps(windowspos, cls=gwnd.EnhancedJSONEncoder))
