# PNG-based PlayerMiniInfo UI Implementation

## Current Task: Implementing PNG Textures for UI

### Files to Load (from Media.pk2 → newui\playerminiinfo\):
- [ ] `mainbackrgound.png` - Main frame/background
- [ ] `HpBar_Health.png` - HP bar fill texture
- [ ] `HpBar_Mana.png` - MP bar fill texture  
- [ ] `hpbar_hwan.png` - Hwan bar fill texture

### UI Layout:
- [ ] Character portrait in circular area (left)
- [ ] Level number in blue circle area (top right of portrait)
- [ ] Character name in red bar area (top right section)
- [ ] HP/MP/Hwan bars below name

### Implementation Steps:
- [ ] Add texture loading system for pk2 files
- [ ] Load PNG textures as D3D9 textures
- [ ] Modify CustomPlayerMiniInfo to render textures
- [ ] Position elements correctly on frame
- [ ] Implement HP/MP bar fill clipping based on percentage
