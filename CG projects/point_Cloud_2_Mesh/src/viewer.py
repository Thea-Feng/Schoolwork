import numpy as np 
import trimesh
import re
with open("../result/input.txt") as f:
    name = f.readline()
    line = f.readline()
    vertices = eval(line)
    # print(type(vertices))
    line = f.readline()
    face = eval(line)
    f.close()

t = re.split(r"[./\"]", name)
mesh = trimesh.Trimesh(vertices=vertices, faces=face, process=False)
mesh.show()
# print(t)
print(mesh)



