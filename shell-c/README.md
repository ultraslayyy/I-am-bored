# I-am-bored

Basic shell written in C.

## How to Build and Run locally
### Prerequisites
- GCC

### Build
Set the `bin` folder as the working directory:
```bash
cd bin
```
**WINDOWS**
```bash
ws_tools build
```

**ON LINUX**
```sh
# Build for Linux
./ws_tools.sh build
```

### Run
Make sure `bin` is the current working directory. Then run
```bash
ws_tools run

# On Linux
./ws_tools run
```
Optionally there is a `-o` flag to open in a new window, like
```bash
ws_tools run -o
```