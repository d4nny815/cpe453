FILESIZE = 1 << 13 # 8KB
FILENAME = "bigfile.txt"

with open(FILENAME, "w") as f:
    f.write("a" * FILESIZE * 2)