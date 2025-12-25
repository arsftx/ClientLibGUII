from dataclasses import dataclass
from dataclasses_json import dataclass_json
import struct

from sro import gwnd

@dataclass_json
@dataclass
class WndPostions:
    unknown: int
    count: int
    resolution_width: int
    resolution_height: int
    postions: list[gwnd.WndPos]

def read(path: str) -> WndPostions:
    with open(path, "rb") as file:
        unknown, = struct.unpack("i", file.read(4))
        if unknown != 3:
            raise ValueError(f"The unknown variable in wndpos.dat isn't what we want: {unknown}")

        count, resolution_width, resolution_height = struct.unpack("iii", file.read(12))

        postions = []
        for _ in range(count):
            x, y = struct.unpack("ii", file.read(8))
            postions.append(gwnd.WndPos(x, y))

    return WndPostions(unknown, count, resolution_width, resolution_height, postions)

def write(path: str, windowspos: WndPostions):
    with open(path, "wb") as f:
        f.write(struct.pack("iiii", windowspos.unknown, 
                                    len(windowspos.postions), 
                                    windowspos.resolution_width, 
                                    windowspos.resolution_height))

        for pos in windowspos.postions:
            f.write(struct.pack("ii", pos.x, pos.y))
