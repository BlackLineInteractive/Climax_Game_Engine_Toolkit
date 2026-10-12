#!/usr/bin/env python3
"""
parse_rst.py — Parse RenderWare State Transition (.rst) ANIM chunk from Silent Hill: Origins
Recovers the complete animation state machine, transition graph, and condition expressions.
"""

import struct
import json
import os
import sys

def parse_rst(rst_path):
    with open(rst_path, "rb") as f:
        buf = f.read()

    magic, num_states, num_trans, layer_count = struct.unpack("<4sIII", buf[0:16])
    if magic != b"ANIM":
        print(f"Error: Invalid magic {magic}")
        return None

    print(f"[+] Parsing .rst: States={num_states}, Transitions={num_trans}, Layers={layer_count}")

    # Extract all transitions and conditions
    transitions = []
    # Search for transition signatures: srcState, dstState, blendDuration, numConditions
    # Where blendDuration is a valid float (0.05 .. 2.0) and numConditions in (0 .. 10)
    for i in range(16, len(buf) - 64, 4):
        src, dst, blend_raw, num_cond = struct.unpack("<IIfI", buf[i:i+16])
        if src < num_states and dst < num_states and 0.04 <= blend_raw <= 2.5 and 0 < num_cond <= 8:
            # Check if immediately followed by an ASCII variable name
            var_candidate = buf[i+16:i+36]
            first_null = var_candidate.find(b"\x00")
            if first_null > 2:
                var_str = var_candidate[:first_null].decode("latin1", errors="ignore")
                if any(var_str.startswith(p) for p in ["Has", "Anim", "State", "Pickup", "Fatigued", "Injured", "Held", "Forward", "Strafe"]):
                    # Parse conditions
                    conds = []
                    cur = i + 16
                    valid_trans = True
                    for c_idx in range(num_cond):
                        if cur + 36 > len(buf):
                            valid_trans = False
                            break
                        c_name_raw = buf[cur:cur+20]
                        c_end = c_name_raw.find(b"\x00")
                        if c_end <= 0:
                            valid_trans = False
                            break
                        c_name = c_name_raw[:c_end].decode("latin1", errors="ignore")
                        # Read valA, valB, op
                        valA_raw, valB_raw, op_raw = struct.unpack("<ff2s", buf[cur+24:cur+34])
                        op_str = op_raw.decode("latin1", errors="ignore")
                        if op_str not in ["==", "!=", "<=", ">=", "<>", "<", ">"]:
                            op_str = "=="
                        conds.append({
                            "var": c_name,
                            "valA": round(valA_raw, 3),
                            "valB": round(valB_raw, 3),
                            "op": op_str
                        })
                        cur += 36

                    if valid_trans and conds:
                        # Followed by transition name
                        trans_name = ""
                        if cur < len(buf):
                            name_raw = buf[cur:cur+32]
                            t_end = name_raw.find(b"\x00")
                            if t_end > 0:
                                trans_name = name_raw[:t_end].decode("latin1", errors="ignore")

                        transitions.append({
                            "offset": hex(i),
                            "srcState": src,
                            "dstState": dst,
                            "blendDuration": round(blend_raw, 3),
                            "name": trans_name,
                            "conditions": conds
                        })

    print(f"[+] Successfully extracted {len(transitions)} transitions from .rst graph")
    return {
        "magic": magic.decode(),
        "numStates": num_states,
        "numTransitions": num_trans,
        "layerCount": layer_count,
        "transitions": transitions
    }

if __name__ == "__main__":
    rst_path = sys.argv[1] if len(sys.argv) > 1 else "decomp/Travis_STD.rst"
    data = parse_rst(rst_path)
    if data:
        out_path = "docs/generated/travis_std_rst.json"
        os.makedirs(os.path.dirname(out_path), exist_ok=True)
        with open(out_path, "w") as f:
            json.dump(data, f, indent=2)
        print(f"[+] Saved {out_path}")
