from fileinfo import FileInfo
import os, re

file_info = FileInfo()

filename_regex           = r"^cm-[0-9\.]+-[0-9]+-.*.zip$"
file_info.ramdisk        = os.path.join('jflte', 'AOSP', 'cyanogenmod.dualboot.cpio')
file_info.patch          = os.path.join('jflte', 'ROMs', 'AOSP', 'cyanogenmod.dualboot.patch')

def matches(filename):
  if re.search(filename_regex, filename):
    return True
  else:
    return False

def print_message():
  print("Detected Cyanogenmod based ROM zip")
  print("Using patched Cyanogenmod ramdisk")

def get_file_info(filename = ""):
  # My ROM has built in dual-boot support
  if "noobdev" in filename:
    print("ROM has built in dual boot support")
    return None

  return file_info