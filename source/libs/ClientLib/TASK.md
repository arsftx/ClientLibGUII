# ECSRO ClientLib - Auto Hunt System Task List

## Completed Features ✅

### 1. Auto Hunt Settings Window (`IFAutoHuntSettings`)
- [x] Custom window for configuring auto-hunt skills and range
- [x] Attack Skills selection panel (drag-drop from learned skills)
- [x] Buff Skills selection panel (drag-drop from learned skills)
- [x] Range slider (100-5000 game units, step 10)
- [x] Scrollbars for skill panels
- [x] Config save/load from `Setting/AutoHunt_<charname>.dat`
- [x] Integration with Macro Window (AutoAttack toggle button)
- [x] **Walk Around checkbox** - Controls patrol system on/off
- [x] **Go Back Center checkbox** - Returns to center when no targets (if Walk Around OFF)
- [x] **Slider thumb sync** - Initial position correctly synced on window open
- [x] **Simplified UI** - Only Auto Berserk, Go Back Center, Walk Around checkboxes
- [x] **Config version 5** - Backward compatible with old versions

### 2. Auto Target Controller (`AutoTargetController`)
- [x] Monster detection using `CICMonster` class check
- [x] Distance calculation from player (2D horizontal distance)
- [x] Range filtering from center position
- [x] Dead/invalid monster filtering (actionState, state, invalidFlag checks)
- [x] Target selection (closest monster first)
- [x] Target packet sending (0x7045)
- [x] Start position saving for range calculation
- [x] **Debug logging** - Target selection, lost target events

### 3. Auto Buff Controller (`AutoBuffController`)
- [x] Selected buff skill tracking from settings
- [x] Active buff detection using `ActiveBuffManager`
- [x] Missing buff detection
- [x] Buff skill casting using native `sub_56BE40`
- [x] Rate limiting (one buff every 2 seconds)

### 4. Auto Attack Skill Controller (`AutoAttackSkillController`)
- [x] Selected attack skill tracking from settings
- [x] **Imbue skill priority** - Imbue skills cast first before regular attacks
- [x] **Imbue buff detection** - Uses `ActiveBuffManager::IsBuffActive()` to check if imbue is needed
- [x] **Per-skill cooldown tracking** - `std::map<DWORD, s_skillLastCastTime>`
- [x] `GetSkillCastTime()` - Read from SkillData +0x44
- [x] `GetSkillCooldown()` - Read from SkillData +0x48
- [x] `IsSkillReady()` - Check if cooldown passed
- [x] `IsCasting()` - Check if animation is playing
- [x] Attack packet sending (0x7074 with target)
- [x] Rate limiting (500ms minimum between casts)
- [x] Buff priority check - Don't attack if buffs are missing
- [x] Imbue time-based lockout (1000ms after imbue cast to prevent spam)
- [x] **Debug logging** - Buffs missing, still casting, no target events
- [x] **5 second casting timeout** - Auto-reset if stuck casting
- [x] **Casting time accumulation fix** - Only update casting end time when not already casting

### 4a. Auto Berserk (NEW)
- [x] **Hwan Point offset** - `Player + 0x6F7` (BYTE, 0-5)
- [x] **Auto-use detection** - When HwanPoint == 5 and checkbox enabled
- [x] **Berserk packet** - Sends opcode `0x70A7` with byte `01`
- [x] **Default ON** - Auto Berserk enabled by default in LoadConfig
- [x] **Console logging** - Hwan Point status and berserk usage

### 4b. Return to Town on Death (NEW)
- [x] **Death detection** - ActionState at offset `+0x1AF` == `0x02`
- [x] **Town return packet** - Sends opcode `0x3053` (no data)
- [x] **3 second delay** - Waits 3s after death detection before sending packet
- [x] **Bot disable** - Disables all controllers and UI toggle on death
- [x] **Checkbox in settings** - "Return to Town on Death" (default ON)
- [x] **Config version 6** - Backward compatible with v1-5

### 4c. Return to Town on Low Potions (NEW)
- [x] **HP Potion detection** - Uses `CIFAutoPotion::Potion_ObjID_HP_Potion_List`
- [x] **MP Potion detection** - Uses `CIFAutoPotion::Potion_ObjID_MP_Potion_List`
- [x] **Pet HP Potion detection** - IDs: 9008 (XL), 2144 (L), 2143 (S)
- [x] **Return Scroll detection** - IDs: 3769 (Mall), 2199, 2198, 61
- [x] **Threshold check** - Low when < 20 potions (`LOW_ITEM_THRESHOLD`)
- [x] **Return scroll priority** - Mall > Scroll_03 > Scroll_02 > Scroll_01
- [x] **Use item packet** - Opcode `0x704C` with TypeID mapping:
  - Mall scroll: `0x09ED`
  - Normal scrolls: `0x09EC`
- [x] **3 second delay** - Waits 3s after low potion detection
- [x] **Quantity validation** - Skips stale slots with `quantity <= 0`
- [x] **Dynamic inventory** - Uses `g_pCICPlayer + 0x13B4` for slot count
- [x] **Checkboxes** - "Town on Low HP/MP/Pet HP (<20)"
- [x] **Periodic logging** - Every 5 seconds shows inventory status

### 4d. Return to Town on Low Arrow (NEW)
- [x] **Arrow detection** - Checks arrows in quiver slot using `EquippedItemManager`
- [x] **Threshold check** - Low when < 50 arrows (`LOW_ARROW_THRESHOLD`)
- [x] **Checkbox** - "Town on Low Arrow (<50)"
- [x] **Config version 8** - Added arrow checkbox to save/load

### 4e. Return to Town on Low Durability (NEW)
- [x] **Current Durability offset** - `CSOItem + 0x68` (verified via IDA `sub_5F5E30`)
- [x] **Max Durability offset** - `CSOItem + 0xA4` (stats + 0x14)
- [x] **Equipment check** - Checks slots 0-7 (Head, Shoulder, Chest, Pants, Hands, Feet, Weapon, Shield)
- [x] **Threshold check** - Low when current durability < 5
- [x] **Periodic check** - Every 15 seconds (`DURABILITY_CHECK_INTERVAL_MS`)
- [x] **Checkbox** - "Town on Low Dura (<5)" - default disabled
- [x] **Config version 9** - Added durability checkbox to save/load

### 5. Auto Move Controller (`AutoMoveController`)
- [x] Walk packet sending (0x7021)
- [x] Center position tracking
- [x] Patrol system (N-S-E-W pattern)
- [x] Range syncing from settings
- [x] Casting lock - Don't move while casting skills
- [x] Buff priority - Don't patrol until all buffs active
- [x] **Walk Around checkbox control** - Patrol only when Walk Around is ON
- [x] **Go Back Center logic** - Return to center when no targets (requires checkbox + Walk Around OFF)
- [x] **Range safety** - MoveTo clamps targets to 80% of range
- [x] **MoveToWithAvoidance range check** - Skip navigation to targets outside range
- [x] **Debug logging** - WalkAround OFF, casting, buffs, target events

### 6. Active Buff Manager (`ActiveBuffManager`)
- [x] Read active buffs from character (+0x800 offset)
- [x] `GetActiveBuffs()` - Returns vector of buff skill IDs
- [x] `IsBuffActive(skillId)` - Check if specific buff is active

### 7. Learned Skill Manager (`LearnedSkillManager`)
- [x] Read learned skills from CIFSkillBoard
- [x] Skill icon path retrieval
- [x] Attack/Buff skill classification by Param1

---

## Known Issues & Fixes Applied 🔧

### Imbue System
- **Problem**: Imbue was spamming (casting multiple times)
- **Fix**: Added 1000ms time-based lockout after imbue cast
- **Fix**: Added `return` after successful imbue cast to prevent regular skills in same Update()

### Skill Casting
- **Problem**: Skills casting too fast, getting "reuse time" error
- **Fix**: Increased cast interval from 300ms to 500ms
- **Fix**: Increased animation buffer from 50ms to 200ms
- **Fix**: Added 200ms fallback for skills without cast time

### Buff Priority
- **Problem**: Attack skills casting before all buffs applied
- **Fix**: Added buff check at very start of Update() before imbue section

### Casting Stuck (NEW)
- **Problem**: Auto hunt getting stuck in "casting" state
- **Fix**: Added 5 second timeout - auto-reset if casting too long
- **Fix**: Only update s_castingEndTime if not already casting (prevents accumulation)

### Range Safety (NEW)
- **Problem**: Patrol would sometimes leave hunt range
- **Fix**: MoveTo clamps targets to 80% of range
- **Fix**: MoveToWithAvoidance skips targets outside range entirely

---

## UI Layout

### Hunt Settings Tab
```
┌─────────────────────────────────────────┐
│ Hunt Settings                            │
├─────────────────────────────────────────┤
│ ☑ Auto Berserk                          │
│ ☑ Return to Town on Death               │
├─────────────────────────────────────────┤
│ Range                                    │
│ ┌───────────────────┬─────────────────┐ │
│ │ [◄──────○──────►] │ ☐ Go Back Center│ │
│ │       100         │ ☑ Walk Around   │ │
│ └───────────────────┴─────────────────┘ │
└─────────────────────────────────────────┘
```

---

### 8. Navmesh & Obstacle Avoidance (`NavMeshManager`)
- [x] Client navmesh/collision data structures reverse engineered
- [x] Access loaded navmesh data for current region
- [x] A* pathfinding around obstacles
- [x] Stuck detection as fallback
- [x] NavMesh visualization for debugging

---

### 9. CSOItem ECSRO Offsets (NEW)
- [x] **Stats base offset** - `CSOItem + 0x90` (via `sub_5F5E40`)
- [x] **Magic Attribute List** - `CSOItem + 0x84` (via `sub_5F7640`)
- [x] **Physical Attack** - `0x90` (Max), `0x94` (Min)
- [x] **Magic Attack** - `0x98` (Max), `0x9C` (Min)
- [x] **Blocking Rate** - `0xA0` (int)
- [x] **Max Durability** - `0xA4` (int)
- [x] **Physical Defense** - `0xA8` (float)
- [x] **Parry Rate** - `0xB0` (float)
- [x] **Critical** - `0xB8` (int)
- [x] **Magic Defense** - `0xBC` (float)
- [x] **Magic Absorption** - `0xC0` (float)
- [x] **Phy Reinforce Min/Max** - `0xC4`, `0xC8` (float %)
- [x] **Mag Reinforce Min/Max** - `0xCC`, `0xD0` (float %)
- [x] **Phy Def Specialize** - `0xD4` (float %)
- [x] **Mag Def Specialize** - `0xD8` (float %)
- [x] **Evasion Rate** - `0xDC` (int)
- [x] **Hit Rate** - `0xE0` (int)
- [x] **Current Durability** - `0x68` (verified via `sub_5F5E30`)

### 10. Equipped Item Manager (`EquippedItemManager`) (NEW)
- [x] `GetEquippedItem(slot)` - Get CSOItem* for equipment slot
- [x] `GetDurability(slot)` - Get current durability
- [x] `GetMaxDurability(slot)` - Get max durability
- [x] `GetDurabilityPercent(slot)` - Get durability percentage
- [x] `HasLowDurabilityEquipment()` - Check if any armor/weapon < 10%
- [x] `LogAllEquippedItems()` - Debug log all stats
- [x] Equipment slots: Helm, Mail, Shoulder, Gauntlet, Pants, Boots, Weapon, Shield, Special, Earring, Necklace, Rings

---

## Key Memory Addresses (ECSRO)

| Address | Description |
|---------|-------------|
| `0xC5DD24` | Game Manager pointer |
| `0xA01010` | Skill Manager |
| `0x56BE40` | UseSkill function (buff casting) |
| `0x668530` | SendWalkPacket function |
| `+0x74` | Entity location (X,Y,Z floats) |
| `+0xE0` | Entity UniqueID |
| `+0x800` | Player buff list |
| `+0x10C` | SkillObject -> SkillData |
| `+0x44` | SkillData cast time |
| `+0x48` | SkillData cooldown |
| `+0x6F7` | **Player Hwan Point** (BYTE, 0-5) |
| `+0x1AF` | **Player ActionState** (0x02 = dead) |
| `0x70A7` | **Hwan USE opcode** (data: 01) |
| `0x3053` | **Town return opcode** (no data) |
| `0x304E` | Hwan Point update packet (type=4) |

### CSOItem Offset Table (ECSRO)
| Offset | Field | Type |
|--------|-------|------|
| `0x2C` | m_refObjItemId | int |
| `0x6C` | m_itemQuantity | int |
| `0x84` | m_MagicAttrList | void* |
| `0x8C` | m_OptLevel | BYTE |
| `0x90` | m_PhyAtkPwrMax | int |
| `0x94` | m_PhyAtkPwrMin | int |
| `0x98` | m_MagAtkPwrMax | int |
| `0x9C` | m_MagAtkPwrMin | int |
| `0xA0` | m_BlockingRateValue | int |
| `0xA4` | m_MaxDurability | int |
| `0xA8` | m_PhyDefPwrValue | float |
| `0xB0` | m_ParryRateValue | float |
| `0xB8` | m_CriticalValue | int |
| `0xBC` | m_MagDefPwrValue | float |
| `0xC0` | m_MagAbsorption | float |
| `0xC4` | m_PhyReinforcementMin | float |
| `0xC8` | m_PhyReinforcementMax | float |
| `0xCC` | m_MagReinforcementMin | float |
| `0xD0` | m_MagReinforcementMax | float |
| `0xD4` | m_PhyDefSpecialize | float |
| `0xD8` | m_MagDefSpecialize | float |
| `0xDC` | m_EvasionRateValue | int |
| `0xE0` | m_AttackRateValue | int |

---

## Imbue Chain IDs
```
Lightning: 237, 238, 239
Cold: 254, 255, 256
Fire: 271, 272, 273
EU: 331, 334, 337
```

---

## File Structure
```
ClientLib/src/
├── AutoAttackSkillController.h/cpp  - Attack skill casting
├── AutoBuffController.h/cpp         - Buff management
├── AutoTargetController.h/cpp       - Monster targeting
├── AutoMoveController.h/cpp         - Movement & patrol
├── ReturnToTownController.h/cpp     - Death/low potion return
├── EquippedItemManager.h/cpp        - Equipment stats reading (NEW)
├── ActiveBuffManager.h/cpp          - Active buff reading
├── LearnedSkillManager.h/cpp        - Skill enumeration
├── IFAutoHuntSettings.h/cpp         - Settings UI window
├── SOItem.h/cpp                     - CSOItem structure (ECSRO offsets)
└── ECSRO_Classes.h                  - Memory access helpers
```

