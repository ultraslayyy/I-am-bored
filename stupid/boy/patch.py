DOS_HEADER_SIZE = 0x40

stub_bytes = bytes([
    0x4C, 0x01,       # mov ax, 0x4c01
    0xCD, 0x21        # int 0x21
])

with open("boy_windows.exe", "rb") as f:
    data = bytearray(f.read())

pe_offset = int.from_bytes(data[0x3C:0x40], "little")

start = DOS_HEADER_SIZE
data[start:start+len(stub_bytes)] = stub_bytes

for i in range(start + len(stub_bytes), pe_offset):
    data[i] = 0x00

with open("boy_windows_patched.exe", "wb") as f:
    f.write(data)