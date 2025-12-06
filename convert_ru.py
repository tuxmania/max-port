import sys

# 1. Check if the user provided filenames
if len(sys.argv) != 3:
    print("Usage: python3 convert_ru.py <input_file> <output_file>")
    sys.exit(1)

input_filename = sys.argv[1]
output_filename = sys.argv[2]

# 2. Define the Transliteration Map (Cyrillic -> Latin)
translit_map = {
    'а': 'a', 'б': 'b', 'в': 'v', 'г': 'g', 'д': 'd', 'е': 'e', 'ё': 'yo',
    'ж': 'zh', 'з': 'z', 'и': 'i', 'й': 'y', 'к': 'k', 'л': 'l', 'м': 'm',
    'н': 'n', 'о': 'o', 'п': 'p', 'р': 'r', 'с': 's', 'т': 't', 'у': 'u',
    'ф': 'f', 'х': 'kh', 'ц': 'ts', 'ч': 'ch', 'ш': 'sh', 'щ': 'shch',
    'ъ': '', 'ы': 'y', 'ь': '', 'э': 'e', 'ю': 'yu', 'я': 'ya',
    'А': 'A', 'Б': 'B', 'В': 'V', 'Г': 'G', 'Д': 'D', 'Е': 'E', 'Ё': 'Yo',
    'Ж': 'Zh', 'З': 'Z', 'И': 'I', 'Й': 'Y', 'К': 'K', 'Л': 'L', 'М': 'M',
    'Н': 'N', 'О': 'O', 'П': 'P', 'Р': 'R', 'С': 'S', 'Т': 'T', 'У': 'U',
    'Ф': 'F', 'Х': 'Kh', 'Ц': 'Ts', 'Ч': 'Ch', 'Ш': 'Sh', 'Щ': 'Shch',
    'Ъ': '', 'Ы': 'Y', 'Ь': '', 'Э': 'E', 'Ю': 'Yu', 'Я': 'Ya'
}

try:
    # 3. Read the UTF-8 file
    with open(input_filename, 'r', encoding='utf-8') as f:
        text = f.read()

    # 4. Transliterate manually
    # If a character isn't in the map, keep it as is
    new_text = "".join(translit_map.get(char, char) for char in text)

    # 5. Save as CP850
    # 'errors=ignore' handles any remaining weird symbols that CP850 can't take
    with open(output_filename, 'w', encoding='cp850', errors='ignore') as f:
        f.write(new_text)
        
    print(f"✅ Success! Converted '{input_filename}' to '{output_filename}'")

except FileNotFoundError:
    print(f"❌ Error: The file '{input_filename}' was not found.")
except Exception as e:
    print(f"❌ Error: {e}")
