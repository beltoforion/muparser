#!/bin/sh

#
# This script is used for building the muParser html page from
# html templates.
#

rm -rf ../mup_*.html
rm -rf ..index.html

#
# add navigation bar to all html templates starting with mup_*
#
cat navigation.html | sed "/\$PLACEHOLDER/r mup_features.html"  | sed "/\$META/r mup_features_meta.html"  > ../mup_features.html
cat navigation.html | sed "/\$PLACEHOLDER/r mup_interface.html" | sed "/\$META/r mup_interface_meta.html" > ../mup_interface.html
cat navigation.html | sed "/\$PLACEHOLDER/r mup_interface_oprt.html" | sed "/\$META/r mup_interface_oprt_meta.html" > ../mup_interface_oprt.html
cat navigation.html | sed "/\$PLACEHOLDER/r mup_intro.html"     | sed "/\$META/r mup_intro_meta.html"     > ../mup_intro.html
cat navigation.html | sed "/\$PLACEHOLDER/r mup_licence.html"   | sed "/\$META/r mup_licence_meta.html"   > ../mup_licence.html
cat navigation.html | sed "/\$PLACEHOLDER/r mup_links.html"     | sed "/\$META/r mup_links_meta.html"     > ../mup_links.html
cat navigation.html | sed "/\$PLACEHOLDER/r mup_usage.html"     | sed "/\$META/r mup_usage_meta.html"     > ../mup_usage.html
cat navigation.html | sed "/\$PLACEHOLDER/r mup_version.html"   | sed "/\$META/r mup_version_meta.html"   > ../mup_version.html
cat navigation.html | sed "/\$PLACEHOLDER/r mup_locale.html"   | sed "/\$META/r mup_locale_meta.html"     > ../mup_locale.html


#for file in mup_*.html
#do
#  echo processing $file
#  cat navigation.html | sed "/\$PLACEHOLDER/r $file" > ../$file
#done

#for file in mup_*_header.html
#do
#  echo processing $file
#  cat navigation.html | sed "/\$META/r $file" > ../$file
#done

# create index.html
cp ../mup_intro.html ../index.html
rm -rf ../*~

