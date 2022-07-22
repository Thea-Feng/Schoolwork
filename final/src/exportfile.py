import numpy as np 
import trimesh
import re
with open("../result/trimesh.txt") as f:
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

dae = trimesh.exchange.dae.export_collada(mesh)
filepath = "../result/" + str(t[-2]) + ".dae"
file = open(filepath,"wb")
file.write(dae)
file.close()
print(" dae file save to ",filepath)

