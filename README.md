k-path
=====

##Build

    make # make release versions
    make debug # make debug versions
    make parallel # make parallel versions
    make serial # make serial version

##Run
    
    ./kpath[xyz] start_vertex finish_vertex final_length graph_file

#### Ex
    
    ./kpath 3 15 12 graph-n35-d.txt
    ./kpath-parallel 3 15 12 graph-n35-d.txt

#### generate graph file

    ./graph_generate.py num_vertices num_edges graph_file
