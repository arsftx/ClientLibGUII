from dataclasses import dataclass
import dataclasses

import json

@dataclass
class WndSize:
    width: int
    height: int

@dataclass
class WndPos:
    x: int
    y: int

@dataclass
class WndRect:
    pos: WndPos
    size: WndSize

class EnhancedJSONEncoder(json.JSONEncoder):
    def default(self, o):
        if dataclasses.is_dataclass(o):
            return dataclasses.asdict(o)
        return super().default(o)