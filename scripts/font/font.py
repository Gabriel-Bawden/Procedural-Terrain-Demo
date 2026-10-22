import os

directory = os.path.dirname(os.path.abspath(__file__))

header_length = 4

header = []
characters = []
id_mapping = []

def get_value(string):
    return int(string.split("=")[1])

with open(directory + "\\font.txt", "r") as file:
    file.readline()
    
    info = file.readline().split()
    
    line_height = get_value(info[1])
    base = get_value(info[2])
    width = get_value(info[3])
    height = get_value(info[4])
    
    file.readline()
    
    character_count = get_value(file.readline())
    
    next_index = -1
    offset = 0
    index_count = 0
    
    for i in range(character_count):
        line = file.readline().split()
        
        char_id = get_value(line[1])
        
        if char_id != next_index:
            if next_index != -1:
                id_mapping.append(next_index - 1)
                id_mapping.append(offset)
                
            id_mapping.append(char_id)
            offset = char_id - index_count
                
        index_count += 1
        next_index = char_id + 1
        
        for j in range(0, 7):
            characters.append(get_value(line[j + 2]))
    
    id_mapping.append(next_index - 1)
    id_mapping.append(offset)
    
    mapping_count = len(id_mapping) // 3
    
    header.append(line_height)
    header.append(base)
    header.append(width)
    header.append(height)
    header.append(character_count)
    header.append(mapping_count)
    
with open(directory + "\\font.fnt", "wb") as file:
    for value in header:
        file.write(value.to_bytes(4, byteorder='little', signed = False))
    for value in characters:
        file.write(value.to_bytes(4, byteorder='little', signed = True))
    for value in id_mapping:
        file.write(value.to_bytes(4, byteorder='little', signed = False))