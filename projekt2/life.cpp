/**
 * @file    life.cpp
 *
 * @author  Šimon Kadnár xkadna00@stud.fit.vutbr.cz
 *
 * @brief   Game Life
 *
 * @date    7.4.2024
 * 
 * @option: implementácia s pevnými stenami (closed)
 **/

#include <mpi.h>
#include <vector>

std::vector<int> str_to_vec(const std::string line){
    std::vector<int> int_line;
    for(char c : line) {
        if(c == '1')
            int_line.push_back(1);
        else if(c == '0')
            int_line.push_back(0);
        else 
        {
            throw std::invalid_argument("Invalid character found in input string.");
        }
    }
    
    return int_line;
}
/*
mriezka je ohranicena
obvod mriezky je vyplneny 0 aby sa lahsie pocitalo ktore bunky maju zostat zit a ktore nie
    000000000
    0mriezka0
    000000000
*/
int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv); 
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Status status;

    std::vector<int> my_line = str_to_vec(argv[rank+1]);
    my_line.insert(my_line.begin(),0);     //vypln
    my_line.push_back(0);                 //vypln

    std::vector<int> up_line;
    std::vector<int> down_line;

    std::string line = argv[rank+1];

    if (rank == 0)  //prvy rank bude mat vzdy vrchne bunky 0
    {
        for (int i = 0; i < line.length()+2; ++i)
            up_line.push_back(0);
    }
    else 
    {
        up_line = str_to_vec(argv[rank]);  
        up_line.insert(up_line.begin(),0);
        up_line.push_back(0);
    }
         

    if (rank == size-1) //posledny rank bude mat vzdy spodne bunky 0
    {        
        for (int i = 0; i < line.length()+2; ++i)
            down_line.push_back(0);
    }
    else 
    {
        down_line = str_to_vec(argv[rank+2]);  
        down_line.insert(down_line.begin(),0);
        down_line.push_back(0);
    }
    int repeat = std::stoi(argv[size+1]);

    for (int i = 0; i < repeat; i++)
    {
        std::vector<int> new_line(1, 0);
        for (int j = 1; j < line.length()+1; j++)
        {
            //vypocet ziviych susdeov
            int sum = up_line[j-1] + up_line[j] + up_line[j+1];
            sum += my_line[j-1] + my_line[j+1];
            sum += down_line[j-1] + down_line[j] + down_line[j+1];
            
            //podmienky pre zvot bunky
            if (my_line[j] == 0 && sum == 3)
                new_line.push_back(1);
            else if ((my_line[j] == 1) && (sum == 2 || sum == 3))
                new_line.push_back(1);
            else
                new_line.push_back(0);
        }
        new_line.push_back(0);  //vypln
        my_line = new_line;

        // posielanie dat
        if (rank != 0) //prvy proces nema komu aktualizovat vrchne bunky
            MPI_Ssend(my_line.data(), my_line.size(), MPI_INT, rank - 1, 0, MPI_COMM_WORLD);

        if (rank != size-1) //poslednemu procesu nema kto aktualizovat spodne bunky
            MPI_Recv(down_line.data(), down_line.size(), MPI_INT, rank + 1, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if (rank != size-1) //posledny proces nema komu aktualizovat spodne bunky
            MPI_Ssend(my_line.data(), my_line.size(), MPI_INT, rank + 1, 0, MPI_COMM_WORLD);

        if (rank != 0)  //prvemu procesu nema kto aktualizovat vrchne bunky
            MPI_Recv(up_line.data(), up_line.size(), MPI_INT, rank - 1, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    //postupny vypis od prveho po posledny rank
    if (rank == 0) 
    {
        std::cout << rank << ": ";
        for (int i = 1; i < my_line.size()-1; i++)
            std::cout << my_line[i];
        std::cout << std::endl;

        MPI_Ssend(NULL, 0, MPI_INT, rank + 1, 2, MPI_COMM_WORLD);
    }
    else 
    {
        MPI_Recv(NULL, 0, MPI_INT, rank - 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if (status.MPI_TAG == 2)
        {
            std::cout << rank << ": ";
            for (int i = 1; i < my_line.size()-1; i++)
                std::cout << my_line[i];
            std::cout << std::endl;
        }
        if( rank != size-1)
            MPI_Ssend(NULL, 0, MPI_INT, rank + 1, 2, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}