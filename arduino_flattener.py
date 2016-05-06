##
# @file       arduino_flattener.py
# @author     Wade Penson
# @date       June, 2015
# @brief      Flattens the folder structure making it easy to use the library
#             with the Arduino IDE.
# @copyright  Copyright 2015 Wade Penson
# 
# @license    Licensed under the Apache License, Version 2.0 (the "License");
#             you may not use this file except in compliance with the License.
#             You may obtain a copy of the License at
# 
#               http://www.apache.org/licenses/LICENSE-2.0
#
#             Unless required by applicable law or agreed to in writing, software
#             distributed under the License is distributed on an "AS IS" BASIS,
#             WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or 
#             implied. See the License for the specific language governing
#             permissions and limitations under the License.


import os
import re
import sys
import shutil

core = (
    "sd_spi_commands\.h",
    "sd_spi_info\.h",
    "sd_spi(\.c|\.h)",
    "arduino_platform_dependencies\.cpp",
    "sd_spi_platform_dependencies\.h",
)

unit_tests = (
    "sd_spi_tests\.c",
    "unit_tests\.ino",
    "planckunit(\.c|\.h)",
    "printf_redirect\.h",
    "serial_c_iface(\.cpp|\.h)"
)

card_info = (
    "card_info\.ino",
)

source_dir = "./"
default_dest_dir = "flatten"

source_paths = []


def copy_files(files, dest_dir):
    for i in range(len(source_paths)):
        if any(re.match(file_name_reg, source_paths[i][1]) for file_name_reg in files):
            source_path = os.path.join(source_paths[i][0], source_paths[i][1])
            dest_path = os.path.join(dest_dir, source_paths[i][1])
            # print('Copied {} to {}'.format(source_path, dest_path))
            shutil.copy(source_path, dest_path)
            fix_includes(dest_path)


def fix_includes(file):
    with open(file) as f:
        out_fname = file + ".tmp"
        out = open(out_fname, "w")
        for line in f:
            out.write(re.sub("#include\s*[\"](?:.*\/|)(.*\.h)\s*\"", "#include \"\g<1>\"", line))
        out.close()
        os.rename(out_fname, file)


for root, dirs, files in os.walk(source_dir):
    for filename in files:
        source_paths.append([root, filename])


# Create output folder
while True:
    temp_dest_dir = raw_input("Output folder (default = flatten/): ")

    if temp_dest_dir == "" or re.match("^\s+$", temp_dest_dir):
        temp_dest_dir = default_dest_dir

    if os.path.isdir(temp_dest_dir):
        print "Folder " + temp_dest_dir + " already exists."
    else:
        try:
                os.makedirs(temp_dest_dir)
                os.makedirs(temp_dest_dir + "/core_files")
                os.makedirs(temp_dest_dir + "/card_info")
                os.makedirs(temp_dest_dir + "/unit_tests")
        except OSError:
            print "Error creating directory."
            continue

        break

# Copy files
copy_files(core, temp_dest_dir + "/core_files")
copy_files(core + card_info, temp_dest_dir + "/card_info")
copy_files(core + unit_tests, temp_dest_dir + "/unit_tests")
