import numpy
import trimesh
import math
filepath = open("tmp.txt").readline()
# filepath = "../data/" + filename
mesh = trimesh.load(filepath)

vertices = mesh.vertices
normals = mesh.vertex_normals
tot = 0
file = open("../result/input.xyz","w")
for i in range(vertices.shape[0]):
    # print(i)
    s = str(vertices[i][0]) + " " + str(vertices[i][1]) + " "+str(vertices[i][2]) + " "+ str(normals[i][0]) + " " + str(normals[i][1]) + " " +str( normals[i][2] )+ "\n"
    file.write(s)

file.close()
