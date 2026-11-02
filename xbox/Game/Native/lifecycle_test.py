"""Exercise the C ABI against a built shared library, without private game inputs."""
import concurrent.futures
import ctypes
import sys
import time

lib = ctypes.CDLL(sys.argv[1])
lib.ps2x_start.argtypes = [ctypes.c_char_p]
lib.ps2x_status.argtypes = [ctypes.c_char_p, ctypes.c_uint32]
lib.ps2x_stop.argtypes = []


def cycle(_):
    for _ in range(5):
        lib.ps2x_start(b"/nonexistent/SCUS_974.72")
        lib.ps2x_stop()


with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
    list(pool.map(cycle, range(4)))
lib.ps2x_stop()
error = ctypes.create_string_buffer(512)
assert lib.ps2x_status(error, len(error)) == 0
for _ in range(3):
    assert lib.ps2x_start(b"/nonexistent/SCUS_974.72") == 1
    deadline = time.monotonic() + 5
    while time.monotonic() < deadline:
        state = lib.ps2x_status(error, len(error))
        if state != 1:
            break
        time.sleep(0.01)
    assert state == -1 and error.value
    lib.ps2x_stop()
    assert lib.ps2x_status(error, len(error)) == 0
print("Native lifecycle: 20 concurrent cycles and 3 error/stop cycles passed")
