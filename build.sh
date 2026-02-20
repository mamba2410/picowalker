# Picowalker-Core
mkdir -p lib
cp ../picowalker-core/build/rp2040/libpicowalker-core.a lib/rp2040
cp ../picowalker-core/build/rp2350/libpicowalker-core.a lib/rp2350


echo "RP2040TouchLCD128 Build"
cd boards/rp2040touchlcd128
rm -rf ../../build/rp2040touchlcd128
cmake -B ../../build/rp2040touchlcd128 -DPICO_BOARD=pico -DUSE_LVGL=ON
cmake --build ../../build/rp2040touchlcd128
cd ../..
echo " "

# echo "RP2350LCD128 Build"
# cd boards/rp2350lcd128
# rm -rf ../../build/rp2350lcd128
# cmake -B ../../build/rp2350lcd128 -DPICO_BOARD=pico2 -DUSE_LVGL=ON
# cmake --build ../../build/rp2350lcd128
# cd ../..

echo "RP2350TouchLCD128 Build"
cd boards/rp2350touchlcd128
rm -rf ../../build/rp2350touchlcd128
cmake -B ../../build/rp2350touchlcd128 -DPICO_BOARD=pico2 -DUSE_LVGL=ON
cmake --build ../../build/rp2350touchlcd128
cd ../..
echo " "

echo "RP2350TouchLCD169 Build"
cd boards/rp2350touchlcd169
rm -rf ../../build/rp2350touchlcd169
cmake -B ../../build/rp2350touchlcd169 -DPICO_BOARD=pico2 -DUSE_LVGL=ON
cmake --build ../../build/rp2350touchlcd169
cd ../..
echo " "