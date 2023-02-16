#!/bin/sh
echo "// Copyright (c) 2023 Adrian \"asie\" Siekierka" > web/resources.js
echo -n "var bin_bootfriend_template = bf_decode_base64(\"" >> web/resources.js
base64 -w 0 bootfriend_template.bin >> web/resources.js
echo "\");" >> web/resources.js
echo -n "var bin_bootfriend_inst_rom = bf_decode_base64(\"" >> web/resources.js
base64 -w 0 installer/bootfriend_inst.wsc >> web/resources.js
echo "\");" >> web/resources.js
