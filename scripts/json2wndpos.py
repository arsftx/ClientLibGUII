from sro import wndpos
import json

settingsfolder = "../BinOut/RelWithDebInfo/setting/"

with open(settingsfolder + "wndpos.json") as f:
    wndpos.write(settingsfolder + "wndpos.dat", 
                 wndpos.WndPostions.from_dict(json.load(f)))
