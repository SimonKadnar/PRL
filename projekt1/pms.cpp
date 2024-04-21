/**
 * @file    pms.cpp
 *
 * @author  Šimon Kadnár xkadna00@stud.fit.vutbr.cz
 *
 * @brief   Pipeline merge sort
 *
 * @date    6.4.2024
 **/

#include <omp.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <queue>

#include <condition_variable>

#include <vector>
#include <mpi.h>
#include <vector>

int pop_val(std::deque<int> &dq) {

if (!dq.empty()) {
        int element = dq.front(); 
        dq.pop_front();
        return element;
    } else {
        std::cerr << "Deque is empty!" << std::endl;
        return -1;
    }
}

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv); 
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Status status;
    
    int TAG = 1;
    bool up_down = false;   // up = true, down = false
    
    std::deque<int> up_queue;   //horny zoznam
    std::deque<int> down_queue; //spodny zoznam

    int up_sequence = 0;        //dlzka hornej sekvencie
    int down_sequence = 0;      //dlzka spodnej sekvencie

    int element;

    if (rank == 0)  //prvy proces
    {
        std::ifstream inputFile("numbers");
        if (!inputFile) 
        {
            std::cerr << "Err while opening file" << argv[1] << std::endl;
            return 1;
        }
        
        unsigned char char_element;
        std::vector<int> int_vec; 
        while (inputFile.read(reinterpret_cast<char*>(&char_element), 1)) 
        {
            std::cout << static_cast<int>(char_element) << " ";
            int_vec.push_back(char_element);
        }
        inputFile.close();
        std::cout << std::endl;

        
        for (int element : int_vec)
        {
            //nastavenie ci dalsia hodnota bude pridana do vrchneho alebo spodneho zoznamu dalsieho procesu
            if (TAG == 0) TAG = 1; else TAG = 0;    

            MPI_Ssend(&element, 1, MPI_INT, 1, TAG, MPI_COMM_WORLD);
        }
        MPI_Ssend(&element, 1, MPI_INT, 1, 2, MPI_COMM_WORLD);  //singalizacia ze prvy proces konci
        inputFile.close();
    }
    else    //ostatne procesy
    { 
        while (true) 
        {
            int rec_data;
            if (status.MPI_TAG != 2)    // ak TAG = 2 posielanie bolo ukoncene  
                MPI_Recv(&rec_data, 1, MPI_INT, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (status.MPI_TAG != 2)   //ak TAG = 2 posedlne poslana hodnota nieje veldina aneprida sa do zoznamu
            {
                if (status.MPI_TAG == 0)    //vyber do ktoreho zo znamov bude pridana hodnota
                {
                    up_queue.push_back(rec_data);
                } 
                else 
                {
                    down_queue.push_back(rec_data);
                }
            }

            //zakldana podmienka pre posielanie hodnot dalsiemu procesu
            if ( ((up_queue.size() >= pow(2, rank-1)) && (down_queue.size() >= 1)) || (status.MPI_TAG == 2) )
            {    
                //pokial sa vycerpa dlzka poslanych dat pre dane nastavenie tag sa prepmne na opacne miesto posielania
                if ( (up_sequence == 0) && (down_sequence == 0) ) 
                {
                    if (TAG == 0) TAG = 1; else TAG = 0;

                    up_sequence = pow(2, rank-1);
                    down_sequence = pow(2, rank-1);
                }

                if (up_sequence == 0)   //pokial horna sekvenica je vycerpana hodnoty sa beru uz iba zo spodnej
                {   
                    up_down = false;
                    down_sequence--;
                }
                else if (down_sequence == 0)    //pokial spodna sekvenica je vycerpana hodnoty sa beru uz iba z vrchnej
                {
                    up_down = true;
                    up_sequence--;
                }
                else    //pokial obe sekvenice niesu vycerane tak je treba porovnat ktora hodnota je vacsia
                {
                    if (up_queue.front() < down_queue.front())
                    {
                        up_down = true;
                        up_sequence--;
                    }
                    else
                    {
                        up_down = false;
                        down_sequence--;
                    }
                }

                if (up_down == true)    //popnutie hodnoty z horneho zoznamu a jej poslanie do dalsieho procesu
                {
                    if (not (up_queue.empty()))
                    {
                        element = pop_val(up_queue);
                        if (rank != size-1) //if this is not last proces send result to next process
                            MPI_Ssend(&element, 1, MPI_INT, rank+1, TAG, MPI_COMM_WORLD); 
                        else                //if this is last process print value on stdout
                            std::cout << element << std::endl;
                    }
                }
                else                    //popnutie hodnoty zo spodneho zoznamu a jej poslanie do dalsieho procesu
                {
                    if (not (down_queue.empty()))
                    {
                        element = pop_val(down_queue);
                        if (rank != size-1)
                            MPI_Ssend(&element, 1, MPI_INT, rank+1, TAG, MPI_COMM_WORLD);
                        else
                            std::cout << element << std::endl;
                    }
                }
            }
            //pokial obydva zoznami su prazdne a dalsie hodnoty uz nepridu, dalsiemu procesu pride informacia o ukonci posielania
            if ( (status.MPI_TAG == 2) && (up_queue.empty()) && (down_queue.empty()))   
            {    
                if (rank != size-1) //posledny proces nema komu poslat informaciu o ukonceni posielania
                    MPI_Ssend(&element, 1, MPI_INT, rank+1, 2, MPI_COMM_WORLD);
                break;
            }
        } 
    }
        
    MPI_Finalize();
    return 0;
}
