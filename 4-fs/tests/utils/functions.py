from datetime import datetime
from subprocess import CompletedProcess
import subprocess
import os
from .constants import MOUNT_POINT

def mount_point_cwd():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    parent_dir = os.path.dirname(script_dir)
    return os.path.join(parent_dir, MOUNT_POINT)   

# subprocess.run("touch file.txt", shell=True)
# subprocess.run(["touch", "file.txt"], shell=False)

def run(
    command: str,
    cwd: str = mount_point_cwd()
) -> CompletedProcess[str]:
    env = os.environ.copy()
    env["LANG"] = "C"
    env["LC_ALL"] = "C"
    return subprocess.run(
        command, 
        shell=True, 
        capture_output=True, 
        text=True, 
        cwd=cwd, 
        env=env
    )

def read_file(path: str) -> str:
    return open(mount_point_cwd() + path).read()

def get_today() -> str:
    return str(datetime.today()).split()[0]