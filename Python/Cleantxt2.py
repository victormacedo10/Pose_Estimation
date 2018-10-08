import numpy as np

with open("corrida.txt", 'r') as infile, open("corrida1.txt", 'w') as outfile:
    temp = infile.read().replace(")],[(", " ")
    temp = temp.replace("]", "")
    temp = temp.replace("[", "")
    temp = temp.replace(")", "")
    temp = temp.replace("(", "")
    temp = temp.replace(",", "")
    temp = temp.replace("None", "-1")
    outfile.write(temp)