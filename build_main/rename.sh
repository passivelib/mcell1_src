#--------------------------------------------
# rename files
#--------------------------------------------
cd cgen_command_line
rename 's/cgen/mcell/g' *.h
rename 's/cgen/mcell/g' *.cpp
cd ..
cd cgen_error
rename 's/cgen/mcell/g' *.h
rename 's/cgen/mcell/g' *.cpp
cd ..
cd cgen_global
rename 's/cgen/mcell/g' *.h
rename 's/cgen/mcell/g' *.cpp
cd ..
cd cgen_technology
rename 's/cgen/mcell/g' *.h
rename 's/cgen/mcell/g' *.cpp
cd ..
cd cgen_components
rename 's/cgen/mcell/g' *.h
rename 's/cgen/mcell/g' *.cpp
cd ..
cd cgen_fast_henry
rename 's/cgen/mcell/g' *.h
rename 's/cgen/mcell/g' *.cpp
cd ..
cd cgen_library
rename 's/cgen/mcell/g' *.h
rename 's/cgen/mcell/g' *.cpp
cd ..
cd cgen_top
rename 's/cgen/mcell/g' *.h
rename 's/cgen/mcell/g' *.cpp
cd ..

#--------------------------------------------
# rename folders
#--------------------------------------------
mv cgen_command_line mcell_command_line
mv cgen_error mcell_error
mv cgen_global mcell_global
mv cgen_technology mcell_technology
mv cgen_components mcell_components
mv cgen_fast_henry mcell_fast_henry
mv cgen_library mcell_library
mv cgen_top mcell_top
