import math
import os

TABLE_SIZE = 4096
AMPLITUDE = 16000

script_dir = os.path.dirname(os.path.abspath(__file__))
FILE_PATH = os.path.join(script_dir, "..", "App", "Inc", "wavetables.h")

def format_table(name, values):
    header = f"const int16_t {name}[{TABLE_SIZE}] = {{\n"
    rows = []
    for i in range(0, len(values), 8):
        line = "    " + ", ".join(map(str, values[i:i+8]))
        rows.append(line)
    footer = "\n};\n\n"
    return header + ",\n".join(rows) + footer

def generate_wavetables():
    sine_values = []
    tri_values = []
    saw_values = []
    sqr_values = []

    for i in range(TABLE_SIZE):
        # 1. SINUS
        sine_values.append(int(round(AMPLITUDE * math.sin(2 * math.pi * i / TABLE_SIZE))))

        # 2. TRIANGLE
        if i < TABLE_SIZE // 4:
            val = (AMPLITUDE * i) // (TABLE_SIZE // 4)
        elif i < 3 * (TABLE_SIZE // 4):
            val = AMPLITUDE - (AMPLITUDE * (i - (TABLE_SIZE // 4))) // (TABLE_SIZE // 4)
        else:
            val = -AMPLITUDE + (AMPLITUDE * (i - 3 * (TABLE_SIZE // 4))) // (TABLE_SIZE // 4)
        tri_values.append(int(round(val)))

        # 3. SAWTOOTH
        val_saw = (2 * AMPLITUDE * i / TABLE_SIZE) - AMPLITUDE
        saw_values.append(int(round(val_saw)))

        # 4. SQUARE
        if i < TABLE_SIZE // 2:
            val_sqr = AMPLITUDE
        else:
            val_sqr = -AMPLITUDE
        sqr_values.append(val_sqr)

    with open(FILE_PATH, "w") as f:
        f.write("#ifndef WAVETABLES_H\n#define WAVETABLES_H\n\n")
        f.write("#include <stdint.h>\n\n")
        
        f.write(format_table("sineLookupTable", sine_values))
        f.write(format_table("triangleLookupTable", tri_values))
        f.write(format_table("sawtoothLookupTable", saw_values))
        f.write(format_table("squareLookupTable", sqr_values))
        
        f.write("#endif\n")

if __name__ == "__main__":
    generate_wavetables()
