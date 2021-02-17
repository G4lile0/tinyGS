import subprocess

revision = ""
try:
    revision = (
        subprocess.check_output(["git", "describe", "--tags", "--always"], stderr=subprocess.DEVNULL)
        .strip()
        .decode("utf-8")
    )
except:
    pass

print("-DGIT_VERSION='\"%s\"'" % revision)