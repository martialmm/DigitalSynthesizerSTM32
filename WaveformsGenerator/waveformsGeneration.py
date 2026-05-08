import math
import os

TABLE_SIZE = 4096
AMPLITUDE = 16000

script_dir = os.path.dirname(os.path.abspath(__file__))
FILE_PATH = os.path.join(script_dir, "..", "App", "Inc", "wavetables.h")

def generate_sine_table():
    with open(FILE_PATH, "w") as f:
        f.write("#ifndef WAVETABLES_H\n#define WAVETABLES_H\n\n")
        f.write("#include <stdint.h>\n\n")
        
        f.write(f"const int16_t sinusLookupTable[{TABLE_SIZE}] = {{\n")
        
        for i in range(TABLE_SIZE):
            value = int(AMPLITUDE * math.sin(2 * math.pi * i / TABLE_SIZE))
            
            # Formatage texte
            f.write(f"    {value},")
            if i % 8 == 7: f.write("\n")            
        f.write("\n};\n\n#endif\n")

generate_sine_table()
