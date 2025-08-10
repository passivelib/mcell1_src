
gdb ./cgen 
start -d 150 -w 5 -s 5 -t transformer2o2 --pin-length=5 --top-metal=TM2 --tech-file-name=ihp130n_tech.txt  --cell-name=test --gds-file-name=test.gds --rect-geometry


./cgen -d 150 -w 5 -s 5 -t transformer2o2 --pin-length=5 --top-metal=TM2 --tech-file-name=ihp130n_tech.txt  --cell-name=test --gds-file-name=test.gds --rect-geometry

./cgen -d 150 -w 5 -s 5 -t transformer2o2 --pin-length=5 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry

./indGen -d 150:200:20 -w 2:10:5 -s 1:5:3 --ss=1:10:5 -t transformer2o2 --pin-length=5 --top-metal=TM2 --tech-file-name=ihp130n_tech.txt  --cell-name=test --gds-file-name=test.gds --rect-geometry --generate-spice-model

./cgen -d 120:200:20 -w 5:10:2 -s 2 --np=5 --ns=1 -t transformer-spiral --pin-space=30 --pin-length=10 --top-metal=TM2 --rect-geometry --generate-spice-model

./cgen -d 200 -w 5 -s 5.1 -n 2 -t inductor-symmetric --pin-length=5  --pin-space=20 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry

./cgen -d 221 -w 5.1 -s 5.125 -n 5 -t inductor-symmetric --pin-length=5  --pin-space=50 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry

./cgen -d 200 -w 7.124 -s 8.12501 -n 5 -t inductor-symmetric --pin-length=5  --pin-space=20 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry


./cgen -d 250 -w 6.12 -s 7.14 -n 7 -t inductor-symmetric --pin-length=10  --pin-space=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry --gnd-shield-metal-name=M1 --gnd-shield-oct-geometry

./cgen -d 150 -w 5 -s 5 -n 2 -t inductor-symmetric --pin-length=10  --pin-space=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry --gnd-shield-metal-name=M1 --gnd-shield-oct-geometry --gnd-shield-diameter=10 --gnd-shield-metal-width=2 --gnd-shield-hole-width=1

./cgen -d 150 -w 5 -s 5 -n 2 -t inductor-spiral --pin-length=10  --pin-space=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry --gnd-shield-metal-name=M1 --gnd-shield-oct-geometry --gnd-shield-diameter=10 --gnd-shield-metal-width=2 --gnd-shield-hole-width=1

./cgen -d 150 -w 5 -s 5 -n 2.5 -t inductor-spiral --pin-length=10  --pin-space=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry --underpass-metal-width=1

./cgen -d 170 -w 5 -s 5 -n 5 -t inductor-spiral --pin-length=10  --pin-space=14.95 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry --underpass-metal-width=1

./cgen -d 140 -w 5 -s 5 -n 6.5 -t inductor-spiral --pin-length=10  --pin-space=2 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --underpass-metal-width=1

./cgen -d 140 -w 5 -s 5 -n 4 -t inductor-spiral --pin-length=10  --pin-space=2 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry --underpass-metal-width=1

./cgen -d 160 -w 5 -s 5 -n 6 -t inductor-spiral --pin-length=10  --pin-space=2 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry --underpass-metal-width=1

cgen -d 100:200:10 -w 5:10:1 -s 5:10:1 -n 2:8:1 -t inductor-spiral --pin-length=10  --pin-space=10 --top-metal=TM2  --oct-geometry --underpass-metal-width=1 --generate-spice-model

cgen -d 100:200:100 -w 5 -s 5 -n 5 -t inductor-spiral --pin-length=10  --pin-space=10 --top-metal=TM2  --oct-geometry --underpass-metal-width=1 --generate-spice-model

./cgen -d 260 -w 5 -s 5 --np=3  --ns=2 -t transformer-spiral --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry

./cgen -d 150 -w 10 -s 5 -n 2 -t inductor-symmetric --pin-length=20 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry

./cgen -d 150 -w 10 -s 5 -n 2 -t inductor-symmetric --pin-length=20 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry --tapped=2

./cgen -d 150 -w 10 -s 5 -n 2 -t inductor-symmetric --pin-length=20 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry --gnd-shield-metal-name=M1 --gnd-shield-oct-geometry --gnd-shield-diameter=10 --gnd-shield-metal-width=2 --gnd-shield-hole-width=1

./cgen -d 150 -w 10 -s 5 -n 2 -t inductor-symmetric --pin-length=20 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry --gnd-shield-metal-name=M1 --gnd-shield-rect-geometry --gnd-shield-diameter=10 --gnd-shield-metal-width=5 --gnd-shield-hole-width=2

./cgen -d 150 -w 10 -s 5 -n 2 -t inductor-symmetric --pin-length=20 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry --gnd-shield-metal-name=M1 --gnd-shield-oct-geometry --gnd-shield-diameter=10 --gnd-shield-metal-width=5 --gnd-shield-hole-width=2


//********************************************************************************************************
//symmetric inductor
//********************************************************************************************************


./cgen -d 150:300:5 -w 5:10:0.5 -s 5:10:0.5 -n 1:10:1 -t inductor-symmetric --pin-length=20  --pin-space=20 --top-metal=TM2 --rect-geometry --gnd-shield-rect-geometry --gnd-shield-metal-name=M1 --generate-em --parallel=0

./cgen -d 150:300:25 -w 5:10:5 -s 5:10:5 -n 1:5:1 -t inductor-symmetric --pin-length=20 --top-metal=TM2 --rect-geometry --generate-em --parallel=0

./cgen -d 150:300:25 -w 5:10:0.5 -s 5:10:0.5 -n 1:5:1 -t inductor-symmetric --pin-length=20 --top-metal=TM2 --rect-geometry --mesh-width=40 --generate-em --parallel=0

./cgen -d 200 -w 10 -s 5 -n 4 -t inductor-symmetric --pin-length=20 --top-metal=TM2 --rect-geometry --mesh-width=40 --faster-cap2 --faster-cap-solve-in-box --faster-cap-box-size=100 --faster-cap-box-hight=150

./cgen -d 150:300:25 -w 5:10:0.5 -s 5:10:0.5 -n 1:10:1 -t inductor-symmetric --pin-length=20 --top-metal=TM2 --rect-geometry --mesh-width=0.5 --fast-henry --parallel=0

./cgen -d 150 -w 5 -s 5 -n 1 -t inductor-symmetric --pin-length=20  --pin-space=20 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry

./cgen -d 150 -w 5 -s 5 -n 1:4:1 -t inductor-symmetric --pin-length=20  --pin-space=20 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry --faster-cap 

./cgen -d 150:200:10 -w 5 -s 5 -n 1:4:1 -t inductor-symmetric --pin-length=20  --pin-space=20 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry --faster-cap --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v81/test/02_complie_laptop

./cgen -d 200 -w 5 -s 5 -n 1:5:1 -t inductor-symmetric --pin-length=20  --pin-space=20 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --generate-em

./cgen -d 200 -w 5 -s 5 -n 1:5:1 -t inductor-symmetric --pin-length=20  --pin-space=20 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry --generate-em

./cgen -d 150:160:10 -w 9:10:1 -s 5 -n 2.5 -t inductor-symmetric --pin-length=20 --top-metal=TM2 --rect-geometry --underpass-metal-width=2 --generate-em --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/02_project_path/a


./cgen -d 200:250:10 -w 8:10:1 -s 5 -n 2.5 -t inductor-symmetric --pin-length=20 --top-metal=TM2 --oct-geometry --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/02_project_path/a --generate-spice-model

./cgen -d 150:160:10 -w 9:10:1 -s 5 -n 2.5 -t inductor-symmetric --pin-length=20 --top-metal=TM2 --rect-geometry --generate-em --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/02_project_path/a --gnd-shield-rect-geometry --gnd-shield-metal-name=M1 --tapped

./cgen -d 200 -w 10 -s 5 -n 2 -t inductor-symmetric --pin-length=20 --top-metal=TM2 --rect-geometry --tapped --fast-henry

./cgen -d 150:200:10 -w 10 -s 5 -n 2 -t inductor-symmetric --pin-length=20 --top-metal=TM2 --rect-geometry --generate-em
./cgen -d 150:200:10 -w 10 -s 5 -n 2 -t inductor-symmetric --pin-length=20 --top-metal=TM2 --rect-geometry --fast-henry --mesh-width=2 -f 1:1e6:10

cgen -d 200 -w 10 -s 5 -n 4 -t inductor-symmetric --pin-length=20 --top-metal=TM2 --rect-geometry --generate-em
./cgen -d 200 -w 10 -s 5 -n 4 -t inductor-symmetric --pin-length=20 --top-metal=TM2 --rect-geometry --mesh-width=10 --faster-cap
./cgen -d 200 -w 10 -s 5 -n 4 -t inductor-symmetric --pin-length=20 --top-metal=TM2 --rect-geometry --fast-henry

./cgen -d 100 -w 10 -s 5 -n 1 -t inductor-symmetric --pin-length=20 --top-metal=TM2 --rect-geometry --faster-cap

./cgen -d 200:400:100 -w 5 -s 5 -n 2 -t inductor-symmetric --pin-length=20 --top-metal=TM2 --oct-geometry --fast-henry2 --mesh-width=1 -f 1e9:10e9:10
./cgen -d 200:400:100 -w 5 -s 5 -n 2 -t inductor-symmetric --pin-length=20 --top-metal=TM2 --oct-geometry --mesh-width=10 --faster-cap
./cgen -d 400 -w 5 -s 5 -n 1 -t inductor-symmetric --pin-length=20 --pin-space=10 --top-metal=TM2 --oct-geometry --mesh-width=10 --faster-cap
./cgen -d 200 -w 5 -s 5 -n 2 -t inductor-symmetric --pin-length=20 --top-metal=TM2 --rect-geometry --fast-henry2 --mesh-width=1 -f 1e9:10e9:10

plEval -i ./yFile/plIndSymOct_d400.00_w5.00_s5.00_n4.00_ps0.00_pl20.00.y -t inductor-diff -f 200e9 -c
plEval -i ./zFile/plIndSymOct_d400.00_w5.00_s5.00_n4.00_ps0.00_pl20.00.out -t inductor-diff --fast-henry -c -r reportFh.txt

//********************************************************************************************************
//spiral inductor
//********************************************************************************************************

./cgen -d 150 -w 10 -s 5 -n 2.5 -t inductor-spiral --pin-length=20 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --underpass-metal-width=2

./cgen -d 150:400:10 -w 5:10:2.5 -s 5:10:2.5 -n 2.5:10:0.25 -t inductor-spiral --pin-length=20 --top-metal=TM2 --rect-geometry --underpass-metal-width=2 --generate-em --gnd-shield-rect-geometry --gnd-shield-metal-name=M1 --parallel=0

./cgen -d 150:300:50 -w 5:10:5 -s 5:10:5 -n 2.5:10:1 -t inductor-spiral --pin-length=20 --top-metal=TM2 --rect-geometry --underpass-metal-width=2 --faster-cap --parallel=0 --mesh-width=40

./cgen -d 150:300:50 -w 5:10:5 -s 5:10:5 -n 2.5:10:1 -t inductor-spiral --pin-length=20 --top-metal=TM2 --rect-geometry --underpass-metal-width=2 --faster-cap2 --parallel=0 --mesh-width=40

./cgen -d 150:400:50 -w 5:10:5 -s 5:10:5 -n 2.5:10:1 -t inductor-spiral --pin-length=20 --top-metal=M5 --rect-geometry --underpass-metal-width=2 --faster-cap --parallel=0 --mesh-width=20

./cgen -d 150:400:50 -w 5:10:5 -s 5:10:5 -n 2.5:10:1 -t inductor-spiral --pin-length=20 --top-metal=TM2 --rect-geometry --underpass-metal-width=2 --faster-cap2 --parallel=0 --mesh-width=20

./cgen -d 150:400:50 -w 5:10:5 -s 5:10:5 -n 2.5:10:1 -t inductor-spiral --pin-length=20 --top-metal=TM2 --rect-geometry --underpass-metal-width=2 --fast-henry --parallel=2 --mesh-width=1

./cgen -d 150:400:50 -w 5:10:5 -s 5:10:5 -n 2.5:10:1 -t inductor-spiral --pin-length=20 --top-metal=TM2 --rect-geometry --underpass-metal-width=2 --fast-henry2 --parallel=0 --mesh-width=1

./cgen -d 150:400:10 -w 5:10:2.5 -s 5:10:2.5 -n 2.5:10:0.25 -t inductor-spiral --pin-length=20 --top-metal=TM2 --rect-geometry --underpass-metal-width=2 --fast-henry --parallel=0 --mesh-width=1

./cgen -d 150 -w 10 -s 5 -n 2 -t inductor-spiral --pin-length=20 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --underpass-metal-width=2

./cgen -d 150 -w 10 -s 5 -n 2.5 -t inductor-spiral --pin-length=20 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry --underpass-metal-width=2

./cgen -d 150 -w 10 -s 5 -n 2 -t inductor-spiral --pin-length=20 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --underpass-metal-width=2

./cgen -d 150:200:10 -w 10 -s 5 -n 2.5 -t inductor-spiral --pin-length=20 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --underpass-metal-width=2 --generate-spice-model

./cgen -d 150 -w 10 -s 5 -n 2.5 -t inductor-spiral --pin-length=20 --top-metal=TM2 --rect-geometry --underpass-metal-width=2 --generate-em

./cgen -d 150 -w 10 -s 5 -n 2.5 -t inductor-spiral --pin-length=20 --top-metal=TM2 --rect-geometry --underpass-metal-width=2 --generate-em --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/02_project_path/a

./cgen -d 150:200:10 -w 5:10:5 -s 5 -n 2.5 -t inductor-spiral --pin-length=20 --top-metal=TM2 --rect-geometry --underpass-metal-width=2 --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/02_project_path/a --generate-spice-model

./cgen -d 150:200:25 -w 10 -s 5 -n 2.5 -t inductor-spiral --pin-length=20 --top-metal=TM2 --rect-geometry --underpass-metal-width=2 --generate-em --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/02_project_path/a --gnd-shield-rect-geometry --gnd-shield-metal-name=M1


./cgen -d 200 -w 10 -s 5 -n 2.5 -t inductor-spiral --pin-length=20 --top-metal=TM2 --rect-geometry --underpass-metal-width=2 --fast-henry

./cgen -d 200 -w 10 -s 5 -n 2.5 -t inductor-spiral --pin-length=20 --top-metal=TM2 --rect-geometry --underpass-metal-width=2 --faster-cap

./cgen -d 200 -w 10 -s 5 -n 2.5 -t inductor-spiral --pin-length=20 --top-metal=TM2 --rect-geometry --underpass-metal-width=2 --generate-em

./cgen -d 150:200:10 -w 10 -s 5 -n 2.5 -t inductor-spiral --pin-length=20 --top-metal=TM2 --rect-geometry --underpass-metal-width=2 --fast-henry
./cgen -d 150:200:10 -w 10 -s 5 -n 2.5 -t inductor-spiral --pin-length=20 --top-metal=TM2 --rect-geometry --underpass-metal-width=2 --generate-em

./cgen -d 150:200:10 -w 10 -s 5 -n 2.5 -t inductor-spiral --pin-length=20 --top-metal=TM2 --rect-geometry --underpass-metal-width=2 --generate-em

./cgen -d 350 -w 5 -s 5 -n 4.5 -t inductor-spiral --pin-length=20 --top-metal=TM2 --rect-geometry --underpass-metal-width=2 --generate-em
./cgen -d 350 -w 5 -s 5 -n 4.5 -t inductor-spiral --pin-length=20 --top-metal=TM2 --rect-geometry --underpass-metal-width=2 --faster-cap2 --mesh-width=10


//********************************************************************************************************
//transformer spiral
//********************************************************************************************************

./cgen -d 180 -w 5 -s 2 --np=3  --ns=2 -t transformer-spiral --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry --tapped-primary=2 --tapped-secondary=2

./cgen -d 150 -w 5 -s 5 --np=3  --ns=2 -t transformer-spiral --pin-length=20 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry

./cgen -d 150 -w 10 -s 5 --np=2  --ns=1 -t transformer-spiral --pin-length=20 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --tapped-primary=2 --tapped-secondary=2

./cgen -d 180 -w 5 -s 2 --np=3  --ns=2 -t transformer-spiral --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --tapped-primary=2 --tapped-secondary=2 --gnd-shield-metal-name=M1 --gnd-shield-oct-geometry --gnd-shield-diameter=10 --gnd-shield-metal-width=5 --gnd-shield-hole-width=2

./cgen -d 180 -w 5 -s 2 --np=3  --ns=2 -t transformer-spiral --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry

./cgen -d 280 -w 5 -s 2 --np=5  --ns=2 -t transformer-spiral --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry

./cgen -d 180 -w 10 -s 2 --np=3  --ns=2 -t transformer-spiral --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry

./cgen -d 250 -w 10 -s 2 --np=5  --ns=2 -t transformer-spiral --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry

./cgen -d 200:220:10 -w 8:10:1 -s 2 --np=5  --ns=2 -t transformer-spiral --top-metal=TM2 --rect-geometry --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/02_project_path/a --generate-spice-model

./cgen -d 250:300:25 -w 8:10:1 -s 2 --np=5  --ns=2 -t transformer-spiral --top-metal=TM2 --rect-geometry --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/02_project_path/a --generate-em

./cgen -d 250:300:25 -w 8:10:1 -s 2 --np=5  --ns=2 -t transformer-spiral --top-metal=TM2 --rect-geometry --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/02_project_path/a --generate-em --tapped-primary=2 --tapped-secondary=2 --gnd-shield-metal-name=M1 --gnd-shield-rect-geometry --gnd-shield-diameter=10 --gnd-shield-metal-width=5 --gnd-shield-hole-width=2

./cgen -d 250:300:25 -w 8:10:1 -s 2 --np=5  --ns=2 -t transformer-spiral --top-metal=TM2 --rect-geometry --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/02_project_path/a --generate-em --gnd-shield-metal-name=M1 --gnd-shield-rect-geometry --tapped-primary=2 --tapped-secondary=2


./cgen -d 300 -w 5 -s 5 --np=5  --ns=2 -t transformer-spiral --top-metal=TM2 --rect-geometry --tapped-primary=2 --tapped-secondary=2 --fast-henry

./cgen -d 150:200:10 -w 5 -s 5 --np=3  --ns=2 -t transformer-spiral --top-metal=TM2 --rect-geometry --fast-henry
./cgen -d 150:200:10 -w 5 -s 5 --np=3  --ns=2 -t transformer-spiral --top-metal=TM2 --rect-geometry --generate-em

./cgen -d 300 -w 5 -s 5 --np=1:5:1  --ns=1:4:1 -t transformer-spiral --top-metal=TM2 --rect-geometry --generate-em

./cgen -d 300 -w 5 -s 5 --np=5  --ns=2 -t transformer-spiral --top-metal=TM2 --oct-geometry --fast-henry
./cgen -d 300 -w 5 -s 5 --np=5  --ns=2 -t transformer-spiral --top-metal=TM2 --oct-geometry --faster-cap
./cgen -d 300 -w 5 -s 5 --np=5  --ns=2 -t transformer-spiral --top-metal=TM2 --oct-geometry --generate-em

./cgen -d 150:400:10 -w 5:10:2.5 -s 5:10:2.5 --np=1:10:1  --ns=1:10:1 -t transformer-spiral --top-metal=TM2 --oct-geometry --gnd-shield-metal-name=M1 --gnd-shield-oct-geometry --generate-em --parallel=0

./cgen -d 150:400:10 -w 5:10:2.5 -s 5:10:2.5 --np=1:10:1  --ns=1:10:1 -t transformer-spiral --top-metal=TM2 --oct-geometry --gnd-shield-metal-name=M1 --gnd-shield-oct-geometry --fast-henry --mesh-width=1 --parallel=0

./cgen -d 150:400:10 -w 5:10:2.5 -s 5:10:2.5 --np=1:10:1  --ns=1:10:1 -t transformer-spiral --top-metal=TM2 --oct-geometry --gnd-shield-metal-name=M1 --gnd-shield-oct-geometry --fast-henry2 --mesh-width=1 --parallel=0

./cgen -d 150:400:50 -w 5:10:5 -s 5:10:5 --np=1:10:1  --ns=1:10:1 -t transformer-spiral --top-metal=TM2 --oct-geometry --gnd-shield-metal-name=M1 --gnd-shield-oct-geometry --faster-cap --mesh-width=50 --parallel=0

./cgen -d 150:400:50 -w 5:10:5 -s 5:10:5 --np=1:10:1  --ns=1:10:1 -t transformer-spiral --top-metal=M5 --oct-geometry --faster-cap --parallel=0

./cgen -d 150:400:10 -w 5:10:2.5 -s 5:10:2.5 --np=1:10:1  --ns=1:10:1 -t transformer-spiral --top-metal=TM2 --oct-geometry --gnd-shield-metal-name=M1 --gnd-shield-oct-geometry --faster-cap2 --parallel=0

//********************************************************************************************************
//transformer1o1
//********************************************************************************************************

./cgen -d 150 -w 10 --ws=5 -t transformer1o1 --sh=10 --pin-length=20 --sp=30 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --tapped-primary=2 --tapped-secondary=2

./cgen -d 120 -w 10 -t transformer1o1 --pin-length=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry
./cgen -d 120 -w 10 -t transformer1o1 --pin-length=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry

./cgen -d 180:200:10 -w 8:10:1 -t transformer1o1 --pin-length=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/02_project_path/a --generate-spice-model

./cgen -d 180:200:10 -w 8:10:1 -t transformer1o1 --pin-length=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/02_project_path/a --generate-em

./cgen -d 180:200:10 -w 8:10:1 -t transformer1o1 --pin-length=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/02_project_path/a --generate-em --gnd-shield-metal-name=M1 --gnd-shield-rect-geometry --tapped-primary=2 --tapped-secondary=2 --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/03_project_path/tt

./cgen -d 150 -w 10 --ws=5 -t transformer1o1 --sh=10 --pin-length=20 --ps=30 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --tapped-primary=2 --tapped-secondary=2 --fast-henry --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/03_project_path/test2

./cgen -d 100:200:10 -w 5:10:2 --ws=5:10:1 -t transformer1o1 --sh=-10:10:1 --pin-length=20 --ps=30 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --tapped-primary=2 --tapped-secondary=2 --fast-henry --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/03_project_path/test2

./cgen -d 150 -w 10 --ws=5 -t transformer1o1 --sh=10 --pin-length=20 --ps=30 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --tapped-primary=2 --tapped-secondary=2 --fast-henry --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/03_project_path/test2

./cgen -d 250 -w 10 --ws=5 -t transformer1o1 --sh=10 --pin-length=20 --ps=30 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry --tapped-primary=2 --tapped-secondary=2 --fast-henry --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/03_project_path/test2

./cgen -d 150:250:10 -w 5:10:1 --ws=5:10:1 -t transformer1o1 --sh=-10:10:1 --pin-length=20 --ps=30 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry --tapped-primary=2 --tapped-secondary=2 --fast-henry --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/03_project_path/test2

./cgen -d 150:200:10 -w 5:10:2 --ws=5:10:2 -t transformer1o1 --sh=-10:10:2 --pin-length=20 --ps=20 --top-metal=TM2 --rect-geometry --fast-henry

./cgen -d 50:200:5 -w 5:10:1 --ws=5:10:1 -t transformer1o1 --sh=-50:50:5 --pin-length=20 --ps=20 --top-metal=TM2 --rect-geometry --fast-henry -f 1e9

./cgen -d 150:200:10 -w 5:10:2 --ws=5:10:2 -t transformer1o1 --sh=-10:10:2 --pin-length=20 --ps=30 --top-metal=TM2 --rect-geometry --generate-em


./cgen -d 100 -w 10 --ws=10 -t transformer1o1 --sh=0 --pin-length=20 --ps=20 --top-metal=TM2 --rect-geometry --fast-henry2

./cgen -d 100:200:10 -w 5:10:1 --ws=5:10:1 --sh=-50:50:5 -t transformer1o1  --ps=20 --ps=20 --top-metal=TM2 --gnd-shield-metal-name=M1 --gnd-shield-rect-geometry --rect-geometry --generate-em --parallel=0

./cgen -d 100:200:25 -w 5:10:2.5 --ws=5:10:2.5 --sh=-50:50:25 -t transformer1o1  --ps=20 --ps=20 --top-metal=TM2 --rect-geometry --faster-cap --mesh-width=10 --parallel=0 

./cgen -d 100:200:25 -w 5:10:2.5 --ws=5:10:2.5 --sh=-50:50:25 -t transformer1o1  --ps=20 --ps=20 --top-metal=TM2 --rect-geometry --faster-cap --mesh-width=10 --parallel=0 

./cgen -d 100:200:10 -w 5:10:1 --ws=5:10:1 --sh=-50:50:5 -t transformer1o1  --ps=20 --ps=20 --top-metal=TM2 --rect-geometry --fast-henry --mesh-width=0.5 --parallel=0

./cgen -d 100:200:10 -w 5:10:1 --ws=5:10:1 --sh=-50:50:5 -t transformer1o1  --ps=20 --ps=20 --top-metal=TM2 --rect-geometry --fast-henry2 --mesh-width=0.5 --parallel=0  

./cgen -d 100 -w 5 --ws=5 -t transformer1o1 --sh=0 --pin-length=20 --ps=20 --top-metal=TM2 --rect-geometry --faster-cap --mesh-width=10
./cgen -d 100 -w 5 --ws=5 -t transformer1o1 --sh=0 --pin-length=20 --ps=20 --top-metal=TM2 --rect-geometry --faster-cap2 --mesh-width=20

./cgen -d 100 -w 5 --ws=5 -t transformer1o1 --sh=0 --pin-length=20 --ps=20 --top-metal=TM2 --rect-geometry --fast-henry2

//********************************************************************************************************
//transformer2o2
//********************************************************************************************************

./cgen -d 150 -w 10 --ws=5 -s 2 --ss=5 -t transformer2o2 --sh=20 --pin-length=20 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --tapped-primary=2 --tapped-secondary=2 --generate-em

./cgen -d 150 -w 10 -s 2 --ws=5 -t transformer2o2 --sh=7 --pin-length=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --tapped-primary=2 --tapped-secondary=2

 
./cgen -d 250 -w 10 -s 2 -t transformer2o2 --pin-length=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry
./cgen -d 250 -w 10 -s 2 -t transformer2o2 --pin-length=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry

./cgen -d 180:200:5 -w 5 -s 5:7:1 -n 2:3:1 -t inductor-symmetric --pin-length=5  --pin-space=20 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --generate-spice-model

./cgen -d 150 -w 10 -s 5 -n 1 -t inductor-symmetric --pin-length=10  --pin-space=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --gnd-shield-metal-name=M1 --gnd-shield-rect-geometry --gnd-shield-diameter=20 --gnd-shield-metal-width=5 --gnd-shield-hole-width=2

./cgen -d 180 -w 10 -s 5 --np=2  --ns=1 -t transformer-spiral --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --tapped-primary=2 --gnerate-em

./cgen -d 180:200:10 -w 8:10:1 --ws=5 -s 2 --ss=5 -t transformer2o2 --sh=0 --pin-length=20 --top-metal=TM2 --rect-geometry --tapped-primary=2 --tapped-secondary=2 --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/02_project_path/a --generate-em

./cgen -d 180:200:10 -w 8:10:1 --ws=5 -s 2 --ss=5 -t transformer2o2 --sh=0 --pin-length=20 --top-metal=TM2 --oct-geometry --tapped-primary=2 --tapped-secondary=2 --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/02_project_path/a --generate-spice-model

./cgen -d 180:200:10 -w 8:10:1 --ws=5 -s 2 --ss=5 -t transformer2o2 --sh=0 --pin-length=20 --top-metal=TM2 --rect-geometry --tapped-primary=2 --tapped-secondary=2 --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/02_project_path/a --generate-em --gnd-shield-metal-name=M1 --gnd-shield-rect-geometry --tapped-primary=2 --tapped-secondary=2

./cgen -d 580:600:10 -w 8:10:1 --ws=10 -s 10 --ss=10 -t transformer2o2 --sh=100 --pin-length=20 --top-metal=TM2 --rect-geometry --tapped-primary=2 --tapped-secondary=2 --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/02_project_path/a --generate-em --gnd-shield-metal-name=M1 --gnd-shield-rect-geometry --tapped-primary=2 --tapped-secondary=2

./cgen -d 250 -w 5 --ws=10 -s 5 --ss=5 --sh=0 -t transformer2o2 --pin-length=20 --top-metal=TM2 --rect-geometry --tapped-primary=2 --tapped-secondary=2 --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/03_fast_henry/a --tapped-primary=2 --tapped-secondary=2 --fast-henry

./cgen -d 150:300:10 -w 5:10:1 --ws=5:10:1 -s 5:10:2 --ss=5:10:2 --sh=-10:10:2 -t transformer2o2 --pin-length=20 --top-metal=TM2 --rect-geometry --tapped-primary=2 --tapped-secondary=2 --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/03_fast_henry/a --tapped-primary=2 --tapped-secondary=2 --fast-henry


./cgen -d 150:300:50 -w 5:10:5 --ws=5:10:5 -s 5:10:5 --ss=5:10:5 --sh=-10:10:5 -t transformer2o2 --pin-length=20 --top-metal=TM2 --rect-geometry --tapped-primary=2 --tapped-secondary=2 --tapped-primary=2 --tapped-secondary=2 --fast-henry
./cgen -d 150:300:50 -w 5:10:5 --ws=5:10:5 -s 5:10:5 --ss=5:10:5 --sh=-10:10:5 -t transformer2o2 --pin-length=20 --top-metal=TM2 --rect-geometry --tapped-primary=2 --tapped-secondary=2 --tapped-primary=2 --tapped-secondary=2 --generate-em

./cgen -d 300 -w 10 --ws=10 -s 10 --ss=5 --sh=10 -t transformer2o2 --pin-length=20 --top-metal=TM2 --rect-geometry --tapped-primary=2 --tapped-secondary=2 --tapped-primary=2 --tapped-secondary=2 --faster-cap2 --mesh-width=40

./cgen -d 150:300:5 -w 5:10:1 --ws=5:10:1 -s 5:10:1 --ss=5:10:1 --sh=-50:50:5 -t transformer2o2 --pin-length=20 --top-metal=TM2 --rect-geometry --fast-henry --freq=10e9

./cgen -d 150:300:50 -w 5:10:2.5 --ws=5:10:2.5 -s 5:10:2.5 --ss=5:10:2.5 --sh=-50:50:10 -t transformer2o2 --pin-length=20 --top-metal=TM2 --rect-geometry --gnd-shield-metal-name=M1 --gnd-shield-rect-geometry --generate-em --parallel=0

./cgen -d 150:300:25 -w 5:10:2.5 --ws=5:10:2.5 -s 5:10:2.5 --ss=5:10:2.5 --sh=-50:50:10 -t transformer2o2 --pin-length=20 --top-metal=TM2 --rect-geometry --gnd-shield-metal-name=M1 --gnd-shield-rect-geometry --fast-henry --parallel=0

./cgen -d 150:300:50 -w 5:10:2.5 --ws=5:10:2.5 -s 5:10:2.5 --ss=5:10:2.5 --sh=-50:50:10 -t transformer2o2 --pin-length=20 --top-metal=TM2 --rect-geometry --gnd-shield-metal-name=M1 --gnd-shield-rect-geometry --faster-cap --mesh-width=40 --parallel=0

./cgen -d 150:300:50 -w 5:10:5 --ws=5:10:5 -s 5:10:5 --ss=5:10:5 --sh=-50:50:10 -t transformer2o2 --pin-length=20 --top-metal=M5 --rect-geometry --gnd-shield-metal-name=M1 --gnd-shield-rect-geometry --faster-cap --mesh-width=40 --parallel=0

cgen -d 150:300:50 -w 5:10:5 --ws=5:10:5 -s 5:10:5 --ss=5:10:5 --sh=-50:50:10 -t transformer2o2 --pin-length=20 --top-metal=M5 --rect-geometry  --faster-cap --parallel=0 --mesh-width=5


//********************************************************************************************************
//transformer1o2
//********************************************************************************************************

./cgen -d 75 -w 10 -s 2 -t transformer1o2 --sh=10 --ws=2 --pin-length=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --tapped-primary=2 --tapped-secondary=2

./cgen -d 75 -w 10 -s 2 -t transformer1o2 --sh=0 --ws=5 --pin-length=5 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry

./cgen -d 150 -w 10 -s 2 -t transformer1o2 --sh=0 --ws=5 --pin-length=5 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry

./cgen -d 150 -w 10 -s 2 -t transformer1o2 --sh=0 --pin-length=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --tapped-primary=2 --tapped-secondary=2

./cgen -d 100:200:20 -w 2:10:2 -s 1:3:1 -t transformer1o2 --sh=0:4:2 --ws=0.5:1.5:0.5 --pin-length=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --tapped-primary=2 --tapped-secondary=2  --generate-spice-model

./cgen -d 100:200:50 -w 2:10:5 -s 1:3:2 -t transformer1o2 --sh=0:4:2 --ws=0.5:1.5:1 --pin-length=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --tapped-primary=2 --tapped-secondary=2  --generate-spice-model

./cgen -d 100:200:50 -w 2:10:5 -s 5 -t transformer1o2 --sh=0 --ws=5 --pin-length=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --tapped-primary=2 --tapped-secondary=2 --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/02_project_path/a --generate-spice-model

./cgen -d 100:200:50 -w 2:10:5 -s 5 -t transformer1o2 --sh=10 --ws=10 --pin-length=10 --top-metal=TM2 --rect-geometry --tapped-primary=2 --tapped-secondary=2 --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/02_project_path/a --generate-em

./cgen -d 100:200:50 -w 2:10:5 -s 5 -t transformer1o2 --sh=10 --ws=10 --pin-length=10 --top-metal=TM2 --rect-geometry --tapped-primary=2 --tapped-secondary=2 --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/02_project_path/a --generate-em --gnd-shield-metal-name=M1 --gnd-shield-rect-geometry --tapped-primary=2 --tapped-secondary=2

./cgen -d 100:200:50 -w 2:10:5 -s 5 -t transformer1o2 --sh=10 --ws=10 --pin-length=10 --top-metal=TM2 --rect-geometry --tapped-primary=2 --tapped-secondary=2 --tapped-primary=2 --tapped-secondary=2 --fast-henry

./cgen -d 200 -w 5 -s 5 -t transformer1o2 --sh=10 --ws=10 --pin-length=10 --top-metal=TM2 --rect-geometry --tapped-primary=2 --tapped-secondary=2 --tapped-primary=2 --tapped-secondary=2 --generate-em

./cgen -d 200 -w 5 -s 5 -t transformer1o2 --sh=10 --ws=10 --pin-length=10 --top-metal=TM2 --rect-geometry --tapped-primary=2 --tapped-secondary=2 --tapped-primary=2 --tapped-secondary=2 --mesh-width=20 --faster-cap2


./cgen -d 150:400:25 -w 5:10:2.5 -s 5:10:2.5 --sh=-50:50:5 --ws=5:10:2.5 -t transformer1o2 --pin-length=10 --top-metal=TM2 --rect-geometry --gnd-shield-metal-name=M1 --gnd-shield-rect-geometry --tapped-primary=2 --tapped-secondary=2 --generate-em --parallel=0

./cgen -d 150:200:50 -w 5:10:5 -s 5:10:5 --sh=-50:50:25 --ws=5:10:5 -t transformer1o2 --pin-length=10 --top-metal=TM2 --rect-geometry --gnd-shield-metal-name=M1 --gnd-shield-rect-geometry --tapped-primary=2 --tapped-secondary=2 --faster-cap --mesh-width=10 --parallel=0 

./cgen -d 150:200:50 -w 5:10:5 -s 5:10:5 --sh=-50:50:25 --ws=5:10:5 -t transformer1o2 --pin-length=10 --top-metal=TM2 --rect-geometry --gnd-shield-metal-name=M1 --gnd-shield-rect-geometry --tapped-primary=2 --tapped-secondary=2 --faster-cap2 --mesh-width=10 --parallel=0 

./cgen -d 150:400:25 -w 5:10:2.5 -s 5:10:2.5 --sh=-50:50:5 --ws=5:10:2.5 -t transformer1o2 --pin-length=10 --top-metal=TM2 --rect-geometry --tapped-primary=2 --tapped-secondary=2 --faster-cap --parallel=0

./cgen -d 150:400:25 -w 5:10:2.5 -s 5:10:2.5 --sh=-50:50:5 --ws=5:10:2.5 -t transformer1o2 --pin-length=10 --top-metal=TM2 --rect-geometry --gnd-shield-metal-name=M1 --gnd-shield-rect-geometry --tapped-primary=2 --tapped-secondary=2 --fast-henry --parallel=0


//********************************************************************************************************
//transformer2o1
//********************************************************************************************************

./cgen -d 150 -w 10 -s 5 -t transformer2o1 --sh=7 --ws=5 --pin-length=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --tapped-primary=2 --tapped-secondary=2

./cgen -d 150 -w 10 -s 2 -t transformer2o1 --sh=0 --ws=0 --pin-length=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --tapped-primary=2 --tapped-secondary=2

./cgen -d 150 -w 10 -s 2 -t transformer2o1 --sh=-50 --pin-length=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --tapped-primary=2 --tapped-secondary=2

./cgen -d 100:200:20 -w 2:10:2 -s 1:3:1 -t transformer1o2 --ss=0:4:2 --ws=0.5:1.5:0.5 --pin-length=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --tapped-primary=2 --tapped-secondary=2  --generate-spice-model

./cgen -d 100:200:50 -w 2:10:5 -s 1:3:2 -t transformer1o2 --ss=0:4:2 --ws=0.5:1.5:1 --pin-length=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --tapped-primary=2 --tapped-secondary=2  --generate-spice-model

./cgen -d 150 -w 10 -s 2 -t transformer2o1 --sh=0 --ws=5 --pin-length=5 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry

./cgen -d 200 -w 10 -s 2 -t transformer2o1 --sh=0 --ws=5 --pin-length=5 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --oct-geometry


./cgen -d 180:200:10 -w 8:10:1 -s 2 -t transformer2o1 --sh=0 --ws=5 --pin-length=5 --top-metal=TM2 --oct-geometry --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/02_project_path/a --generate-em


./cgen -d 180:200:10 -w 8:10:1 -s 2 -t transformer2o1 --sh=0 --ws=5 --pin-length=5 --top-metal=TM2 --rect-geometry --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/02_project_path/a --generate-spice-model


./cgen -d 180:200:10 -w 8:10:1 -s 2 -t transformer2o1 --sh=0 --ws=5 --pin-length=5 --top-metal=TM2 --oct-geometry --project-path=/home/icuser/Desktop/projects/cgen/cgen/0v2/test/02_project_path/a --generate-em --gnd-shield-metal-name=M1 --gnd-shield-rect-geometry --tapped-primary=2 --tapped-secondary=2

./cgen -d 180:200:10 -w 8:10:1 -s 2 -t transformer2o1 --sh=0 --ws=5 --pin-length=5 --top-metal=TM2 --oct-geometry --tapped-primary=2 --tapped-secondary=2 --generate-spice-model


./cgen -d 180:200:10 -w 8:10:1 -s 2 -t transformer2o1 --sh=0 --ws=5 --pin-length=5 --top-metal=TM2 --oct-geometry --tapped-primary=2 --tapped-secondary=2 --fast-henry

./cgen -d 180:200:10 -w 8:10:1 -s 2 -t transformer2o1 --sh=0 --ws=5 --pin-length=5 --top-metal=TM2 --oct-geometry --tapped-primary=2 --tapped-secondary=2 --generate-em

./cgen -d 250 -w 5 -s 2 -t transformer2o1 --sh=50 --ws=5 --pin-length=5 --top-metal=TM2 --oct-geometry --tapped-primary=2 --tapped-secondary=2 --fast-henry


./cgen -d 150:400:25 -w 5:10:2.5 -s 2:10:2  --sh=-50:50:5 --ws=5:10:2.5 -t transformer2o1 --pin-length=5 --top-metal=TM2 --oct-geometry --tapped-primary=2 --tapped-secondary=2 --gnd-shield-metal-name=M1 --gnd-shield-oct-geometry --generate-em --parallel=0

./cgen -d 150:400:25 -w 5:10:2.5 -s 2:10:2  --sh=-50:50:5 --ws=5:10:2.5 -t transformer2o1 --pin-length=5 --top-metal=TM2 --oct-geometry --tapped-primary=2 --tapped-secondary=2 --gnd-shield-metal-name=M1 --gnd-shield-oct-geometry --fast-henry --parallel=0

./cgen -d 150:400:50 -w 5:10:5 -s 2:10:5  --sh=-50:50:10 --ws=5:10:5 -t transformer2o1 --pin-length=5 --top-metal=TM2 --oct-geometry --tapped-primary=2 --tapped-secondary=2 --gnd-shield-metal-name=M1 --gnd-shield-oct-geometry --faster-cap --mesh-width=20 --parallel=0


//********************************************************************************************************
//transformer1o1
//********************************************************************************************************

./cgen -d 150 -w 10 -t transformer1o1 --ss=0 --ws=2 --pin-length=10 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --tapped-primary=2 --tapped-secondary=2

./cgen -d 150 -w 10 -t transformer1o1 --sh=20 --ws=2 --pl=50 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --tapped-primary=2 --tapped-secondary=2

./cgen -d 150:200:10 -w 5:10:5 -t transformer1o1 --sh=0 --ws=2 --pl=50 --top-metal=TM2  --cell-name=test --gds-file-name=test.gds --rect-geometry --tapped-primary=2 --tapped-secondary=2

