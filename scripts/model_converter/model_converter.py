# -*- coding: utf-8 -*-
"""
Created on Wed Jul 19 07:38:30 2023

@author: 12673
"""

import struct
import os
from operator import itemgetter

directory = os.path.dirname(os.path.abspath(__file__))

class material:
    def __init__(self):
        self.start_index = 0
        self.ka = []
        self.kd = []
        self.ks = []
        self.illum = 0
        self.ns = 0
        self.map_kd = ""
        self.map_bump = ""

class model:
        def __init__(self):
            self.mod = 0
            self.ind = 0
            self.v_s = 0
            self.has_text = 0
            self.has_norm = 0
            self.material_count = 0
            self.indices = []
            self.vert = []
            self.text = []
            self.norm = []
            self.materials = []
            self.type = ""
            
        def load_mdl(self, path):
            self.type = "mdl"
            self.mod = path.split("\\")[-1].split(".")[0]
            
            with open(path, "rb") as file:
                data = file.read()

                size = 4
                index = size * (5)
                header = struct.unpack("iiiii", data[0:index])
                
                self.ind = header[0]
                self.v_s = header[1]
                self.has_text = header[2]
                self.has_norm = header[3]
                self.material_count = header[4]
                
                if self.v_s:
                    self.indices = list(struct.unpack(str(3 * self.ind) + "i", data[index:index + size * (3 * self.ind)]))
                    index += size * (3 * self.ind)
                    
                    self.vert = list(struct.unpack(str(3 * self.v_s) + "f", data[index:index + size * (3 * self.v_s)]))
                    index += size * (3 * self.v_s)
                    
                if self.has_text:
                    self.text = list(struct.unpack(str(2 * self.v_s) + "f", data[index:index + size * (2 * self.v_s)]))
                    index += size * (2 * self.v_s)
                    
                if self.has_norm:
                    self.norm = list(struct.unpack(str(3 * self.v_s) + "f", data[index:index + size * (3 * self.v_s)]))
                    index += size * (3 * self.v_s)
                    
                if self.material_count:
                    for i in range(self.material_count):
                        new_mat = material()
                        
                        new_mat.start_index = struct.unpack("i", data[index: index + size * 1])[0]
                        index += size * 1
                        
                        new_mat.ka = list(struct.unpack("3f", data[index:index + size * 3]))
                        index += size * 3
                        
                        new_mat.kd = list(struct.unpack("3f", data[index:index + size * 3]))
                        index += size * 3
                        
                        new_mat.ks = list(struct.unpack("3f", data[index:index + size * 3]))
                        index += size * 3
                        
                        new_mat.illum = struct.unpack("i", data[index:index + size * 1])[0]
                        index += size * 1
                        
                        new_mat.ns = struct.unpack("f", data[index:index + size * 1])[0]
                        index += size * 1
                        
                        new_mat.map_kd = struct.unpack("50s", data[index:index + 1 * 50])[0].decode("utf-8").replace("\0", "")
                        index += 1 * 50
                        
                        new_mat.map_bump = struct.unpack("50s", data[index:index + 1 * 50])[0].decode("utf-8").replace("\0", "")
                        index += 1 * 50
                        
                        self.materials.append(new_mat)
                    
        def load_obj(self, path):
            self.type = "obj"
            self.mod = path.split("\\")[-1].split(".")[0]
            
            ind_temp = 0
            v_s_temp = 0
            has_text_temp = 0
            has_norm_temp = 0
            indices_temp = []
            vert_temp = []
            text_temp = []
            norm_temp = []
            
            materials = []
            groups = []
            
            with open(path, "r") as file:
                for line in file:
                    line = line.strip()
                    
                    if line[0:2] == "v ":
                        vals = line[2:].split()

                        vert_temp.append([float(vals[0]), float(vals[1]), float(vals[2])])
                    elif line[0:3] == "vt ":
                        vals = line[3:].split()
                        
                        text_temp.append([float(vals[0]), float(vals[1])])
                    elif line[0:3] == "vn ":
                        vals = line[3:].split()
                        
                        norm_temp.append([float(vals[0]), float(vals[1]), float(vals[2])])
                    elif line[0:2] == "f ":
                        ind_temp += 1
                        
                        if "//" in line:
                            str = line[2:].replace(" ", "//")
                            vals = str.split("//")

                            indices_temp.append([int(vals[0]) - 1, int(vals[1]) - 1])
                            indices_temp.append([int(vals[2]) - 1, int(vals[3]) - 1])
                            indices_temp.append([int(vals[4]) - 1, int(vals[5]) - 1])
                        else:
                            if line.count("/") == 3:
                                str = line[2:].replace("/", " ")
                                vals = str.split()

                                indices_temp.append([int(vals[0]) - 1, int(vals[1]) - 1])
                                indices_temp.append([int(vals[2]) - 1, int(vals[3]) - 1])
                                indices_temp.append([int(vals[4]) - 1, int(vals[5]) - 1])
                            elif line.count("/") == 6:
                                str = line[2:].replace("/", " ")
                                vals = str.split()

                                indices_temp.append([int(vals[0]) - 1, int(vals[1]) - 1, int(vals[2]) - 1])
                                indices_temp.append([int(vals[3]) - 1, int(vals[4]) - 1, int(vals[5]) - 1])
                                indices_temp.append([int(vals[6]) - 1, int(vals[7]) - 1, int(vals[8]) - 1])
                            else:
                                vals = line[2:].split()
                                
                                indices_temp.append([int(vals[0]) - 1])
                                indices_temp.append([int(vals[1]) - 1])
                                indices_temp.append([int(vals[2]) - 1])
                    elif line[0:7] == "mtllib ":
                        material_path = path[:path.rfind("\\") + 1] + line[7:]

                        with open(material_path.strip(), "r") as material_file:
                            new_mat = 0
                            
                            for mat_line in material_file:
                                mat_line = mat_line.strip().split()
                                
                                if len(mat_line) > 0:
                                    if mat_line[0] == "newmtl":
                                        if new_mat:
                                            self.materials.append(new_mat)
                                            
                                        materials.append(mat_line[1])
                                        
                                        new_mat = material()
                                    elif mat_line[0] == "Ka":
                                        new_mat.ka.append(float(mat_line[1]))
                                        new_mat.ka.append(float(mat_line[2]))
                                        new_mat.ka.append(float(mat_line[3]))
                                    elif mat_line[0] == "Kd":
                                        new_mat.kd.append(float(mat_line[1]))
                                        new_mat.kd.append(float(mat_line[2]))
                                        new_mat.kd.append(float(mat_line[3]))
                                    elif mat_line[0] == "Ks":
                                        new_mat.ks.append(float(mat_line[1]))
                                        new_mat.ks.append(float(mat_line[2]))
                                        new_mat.ks.append(float(mat_line[3]))
                                    elif mat_line[0] == "illum ":
                                        new_mat.illum = int(mat_line[1])
                                    elif mat_line[0] == "Ns":
                                        new_mat.ns = float(mat_line[1])
                                    elif mat_line[0] == "map_Kd" and len(mat_line) > 1:
                                        map_kd = mat_line[1]
                                        
                                        if len(map_kd) > 4 and map_kd[-4] == ".":
                                            map_kd = map_kd.split(".")[-2]
                                        
                                        if "\\" in map_kd:
                                            map_kd = map_kd.split("\\")[-1]
                                            
                                        if "/" in map_kd:
                                            map_kd = map_kd.split("/")[-1]
                                            
                                        new_mat.map_kd = map_kd
                                    elif mat_line[0] == "map_Bump" or mat_line[0] == "bump" and len(mat_line) > 1:
                                        map_bump = mat_line[1]
                                        
                                        if mat_line[1] == "-bm": #Should store in material
                                            map_bump = mat_line[3]
                                        
                                        if len(map_bump) > 4 and map_bump[-4] == ".":
                                            map_bump = map_bump.split(".")[-2]
                                        
                                        if "\\" in map_bump:
                                            map_bump = map_bump.split("\\")[-1]
                                            
                                        if "/" in map_bump:
                                            map_bump = map_bump.split("/")[-1]
                                            
                                        new_mat.map_bump = map_bump
                                    
                            self.materials.append(new_mat)
                    elif line[0:7] == "usemtl ":
                        groups.append([materials.index(line[7:]), [ind_temp]])
            
            #Reordering all faces so that all faces of one material type are together
            #The face groups are in the same order as the materials
            self.material_count = len(self.materials)
            
            if self.material_count:
                for i in range(len(groups) - 1):
                    groups[i][1].append(groups[i + 1][1][0] - 1)
                    
                groups[len(groups) - 1][1].append(ind_temp - 1)
                groups = sorted(groups, key=itemgetter(0))
                
                indices_temp_sorted = []
                start_index = 0
                current = -1
                
                for group in groups:
                    if group[0] != current:
                        self.materials[group[0]].start_index = start_index
                        current = group[0]
                        
                    for i in range(group[1][0], group[1][1] + 1):
                        indices_temp_sorted.append(indices_temp[3 * i + 0])
                        indices_temp_sorted.append(indices_temp[3 * i + 1])
                        indices_temp_sorted.append(indices_temp[3 * i + 2])
                        start_index += 1
                
                indices_temp = indices_temp_sorted
                
            #Ensuring that there are equal numbers of positional, texture, and normal vertices

            v_s_temp = len(vert_temp)
            has_text_temp = len(text_temp)
            has_norm_temp = len(norm_temp)
            
            unique_vertices = []
            print("Loaded, combining vertices")
            test_total_count = -1;
            
            for index in indices_temp:
                test_total_count += 1
                if test_total_count % 1000 == 0:
                    print(test_total_count, "/", len(indices_temp))
                
                vertex = []
                
                if has_text_temp and has_norm_temp:
                    vertex = [vert_temp[index[0]], text_temp[index[1]], norm_temp[index[2]]]
                elif has_text_temp:
                    vertex = [vert_temp[index[0]], text_temp[index[1]]]
                elif has_norm_temp:
                    vertex = [vert_temp[index[0]], norm_temp[index[1]]]
                else:
                    vertex = [vert_temp[index[0]]]
                    
                if vertex not in unique_vertices:
                    if v_s_temp:
                        for v in vertex[0]:
                            self.vert.append(v)
                        
                    if has_text_temp:
                        for v in vertex[1]:
                            self.text.append(v)
                            
                    if has_norm_temp:
                        norms = vertex[1]
                        
                        if has_text_temp:
                            norms = vertex[2]
                            
                        for v in norms:
                            self.norm.append(v)
                            
                    self.indices.append(len(unique_vertices))
                    
                    unique_vertices.append(vertex)
                else:
                    self.indices.append(unique_vertices.index(vertex))
                    
            self.ind = len(self.indices) // 3
            self.v_s = len(self.vert) // 3
            self.has_text = len(self.text) > 0
            self.has_norm = len(self.norm) > 0
            
        def save_mdl(self, path):
            if self.type != "mdl":
                self.switch_z_order()
                self.switch_norm_z_order()
            
            with open(path + self.mod + ".mdl", "wb") as file:
                endian = "<"
                
                file.write(struct.pack(endian + "iiiii", self.ind, self.v_s, self.has_text, self.has_norm, self.material_count))
                
                if self.v_s:
                    file.write(struct.pack(endian + str(3 * self.ind) + "i", *self.indices))
                    file.write(struct.pack(endian + str(3 * self.v_s) + "f", *self.vert))
                         
                if self.has_text:
                    file.write(struct.pack(endian + str(2 * self.v_s) + "f", *self.text))
                    
                if self.has_norm:
                    file.write(struct.pack(endian + str(3 * self.v_s) + "f", *self.norm))
                    
                if self.material_count:
                    for mat in self.materials:
                        if len(mat.ka) == 0:
                            mat.ka = [1, 1, 1]
                        if len(mat.kd) == 0:
                            mat.kd = [1, 1, 1]
                        if len(mat.ks) == 0:
                            mat.ks = [1, 1, 1]
                        
                        file.write(struct.pack(endian + "i", mat.start_index))
                        
                        file.write(struct.pack(endian + "3f", *mat.ka))
                                   
                        file.write(struct.pack(endian + "3f", *mat.kd))
                                              
                        file.write(struct.pack(endian + "3f", *mat.ks))
                                   
                        file.write(struct.pack(endian + "i", mat.illum))
                                  
                        file.write(struct.pack(endian + "f", mat.ns))

                        map_kd = mat.map_kd
                        
                        if len(map_kd) > 49:
                            map_kd = map_kd[0:49]
                            map_kd += "\0"
                            
                        file.write(struct.pack(endian + "50s", map_kd.encode('UTF-8')))
                        
                        map_bump = mat.map_bump
                        
                        if len(map_bump) > 49:
                            map_bump = map_bump[0:49]
                            map_bump += "\0"
                            
                        file.write(struct.pack(endian + "50s", map_bump.encode('UTF-8')))

        def save_obj(self, path):
            if self.type != "obj":
                self.switch_z_order()
                self.switch_norm_z_order()
            
            with open(path + self.mod + ".obj", "w") as file:
                if self.material_count:
                    file.write("mtllib " + self.mod + ".mtl\n")
                
                for i in range(self.v_s):
                    if self.v_s:
                        file.write("v " + str(self.vert[3 * i + 0]) + " " + str(self.vert[3 * i + 1]) + " " +  str(self.vert[3 * i + 2]) + "\n")
                for i in range(self.v_s):
                    if self.has_text:
                        file.write("vt " + str(self.text[2 * i + 0]) + " " + str(self.text[2 * i + 1]) + "\n")
                for i in range(self.v_s):    
                    if self.has_norm:
                        file.write("vn " + str(self.norm[3 * i + 0]) + " " + str(self.norm[3 * i + 1]) + " " + str(self.norm[3 * i + 2]) + "\n")

                mat_next = 0
                
                for i in range(self.ind):
                    if mat_next < len(self.materials) and i == self.materials[mat_next].start_index:
                        file.write("usemtl " + str(mat_next) + "\n")
                        mat_next += 1

                    f1 = str(self.indices[3 * i + 0] + 1)
                    f2 = str(self.indices[3 * i + 1] + 1)
                    f3 = str(self.indices[3 * i + 2] + 1)
                    
                    if self.has_text and self.has_norm:
                        file.write("f " + f1 + "/" + f1 + "/" + f1 + " " + f2 + "/" + f2 + "/" + f2 + " " + f3 + "/" + f3 + "/" + f3 + "\n")
                    elif self.has_text:
                        file.write("f " + f1 + "/" + f1 + " " + f2 + "/" + f2 + " " + f3 + "/" + f3 + "\n")
                    elif self.has_norm:
                        file.write("f " + f1 + "//" + f1 + " " + f2 + "//" + f2 + " " + f3 + "//" + f3 + "\n")
                    else:
                        file.write("f " + f1 + " " + f2 + " " + f3 + "\n")
            
            if self.material_count:
                with open(path + self.mod + ".mtl", "w") as file:
                    for i, mat in enumerate(self.materials):
                        file.write("newmtl " + str(i) + "\n")
                        
                        file.write("Ka " + str(mat.ka[0]) + " " + str(mat.ka[1]) + " " + str(mat.ka[2]) + "\n")
                        
                        file.write("Kd " + str(mat.kd[0]) + " " + str(mat.kd[1]) + " " + str(mat.kd[2]) + "\n")
                        
                        file.write("Ks " + str(mat.ks[0]) + " " + str(mat.ks[1]) + " " + str(mat.ks[2]) + "\n")
                        
                        file.write("illum " + str(mat.illum) + "\n")
                        
                        file.write("Ns " + str(mat.ns) + "\n")
                        
                        file.write("map_Kd " + mat.map_kd + "\n")
                        
                        file.write("map_Bump " + mat.map_bump + "\n")
                        
                        file.write("\n")
                        
        def switch_winding_order(self):
            for i in range(self.ind):
                temp = self.indices[3 * i + 0]
                self.indices[3 * i + 0] = self.indices[3 * i + 2]
                self.indices[3 * i + 2] = temp
                
        def switch_z_order(self):
            for i in range(self.v_s):
                self.vert[3 * i + 2] *= -1
                
        def switch_norm_z_order(self):
            if self.has_norm:
                for i in range(self.v_s):
                    self.norm[3 * i + 2] *= -1

mdl_directory = os.listdir(directory + "\\mdl_in")

for path in mdl_directory:
    mod = model()
    
    print("Start: " + path)
    mod.load_mdl(directory + "\\mdl_in\\" + path)
    print("Loaded: " + path + "\n")
    
    #if(mod.mod == "bobomb" or mod.mod == "test"):
        #mod.switch_winding_order()
            
    mod.save_obj(directory + "\\obj_out\\")
    mod.save_mdl(directory + "\\mdl_out\\")

obj_directory = os.listdir(directory + "\\obj_in") 

for path in obj_directory:
    mod = model()
    
    if path[-4:] == ".obj":
        
        print("Start: " + path)
        mod.load_obj(directory + "\\obj_in\\" + path)
        print("Loaded: " + path + "\n")
        
        #if (mod.mod == "bobomb" or mod.mod == "stalkyard" or mod.mod == "hazymaze" or mod.mod == "whompfortress" or mod.mod == "anomalous" or mod.mod == "z" or mod.mod == "frog"):
            #mod.switch_winding_order()
            
        if (mod.mod == "square_norm"):
            mod.switch_winding_order()
            
        mod.save_mdl(directory + "\\mdl_out\\")
        #mod.save_obj(directory + "\\obj_out\\")
        
print("Finished.")