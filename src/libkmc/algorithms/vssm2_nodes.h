/*
 * Copyright 2009-2013 The VOTCA Development Team (http://www.votca.org)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef __VOTCA_KMC_VSSM2_NODES_H_
#define __VOTCA_KMC_VSSM2_NODES_H_

#include <unordered_map>
#include <time.h>
#include <votca/kmc/algorithm.h>
#include <votca/kmc/bnode.h>
#include "../events/carrier_escape.h"
#include "../events/electron_transfer.h"
#include "../events/hole_transfer.h"


/* Two-level VSSM algorithm with nodes on level 1 and events on level 2
//          head
//        /  |  \
//  node_1      node_n   
//  /  |  \
// move_1  move_k
*/

namespace votca { namespace kmc {
  
class VSSM2_NODES : public Algorithm {
    
public:
    
/*void progressbar(double fraction)
{
    int totalbars = 50;
    std::cout << "\r";
    for(double bars=0; bars<=(totalbars); bars++)
    {
        if(bars<=fraction*(totalbars))
        {
            std::cout << "|";
        }
        else
        {
            std::cout << "-";
        }
    }
    std::cout << "  " << int(fraction*1000)/10. <<" %   ";
    std::cout << std::flush;
    if(fraction*100 == 100)
    {
        std::cout << std::endl;
    }
}*/

void Initialize ( State* _state, Graph* _graph ) { 
    
    state = _state;
    graph = _graph;
    
    // Map of charge transfer events associated with a particular node
    std::unordered_map< BNode*, std::vector<Event*> > electron_transfer_map;
    std::unordered_map< BNode*, std::vector<Event*> > hole_transfer_map;
    
    // Vector of all charge transfer events
    std::vector<ElectronTransfer*> et_events;
    std::vector<HoleTransfer*> ht_events;

    // Create all possible transfer events and associate them with the nodes
    for (Graph::iterator it_node = _graph->nodes_begin(); it_node != _graph->nodes_end(); ++it_node) {
        
        BNode* node_from = *it_node;
        
        // create a new key with an empty vector
        electron_transfer_map.emplace(node_from, vector<Event*>() );
        hole_transfer_map.emplace(node_from, vector<Event*>() );
        
        // Loop over all neighbours (edges) of the node 
        for (BNode::EdgeIterator it_edge = node_from->EdgesBegin(); it_edge != node_from->EdgesEnd(); ++it_edge) {

            
            Event* event_move_hole = Events().Create("hole_transfer");
            HoleTransfer* hole_transfer = dynamic_cast<HoleTransfer*> (event_move_hole);
            hole_transfer->Initialize(NULL, *it_edge);
                        
            // Add a list of hole transfer events to the map, indexed by a node pointer
            hole_transfer_map.at(node_from).push_back(event_move_hole);
            
            // Add an event to the hole transfer events
            ht_events.push_back(hole_transfer);
            //std::cout << "Hole transfer created " << node_from->id << " -> " << (*it_edge)->NodeTo()->id << std::endl;
            
            Event* event_move_electron = Events().Create("electron_transfer");
            ElectronTransfer* electron_transfer = dynamic_cast<ElectronTransfer*> (event_move_electron);
            electron_transfer->Initialize(NULL, *it_edge);
                        
            // Add a list of electron transfer events to the map, indexed by a node pointer
            electron_transfer_map.at(node_from).push_back(event_move_electron);
            
            // Add an event to the electron transfer events
            et_events.push_back(electron_transfer);           
            
        }
    }
    
    
    // for every event, add a list of "events-to-enable" after OnExecute
    // and a list of "events-to-disable" after OnExecute
    for (auto& event: ht_events ) {
        BNode* node_from = event->NodeFrom();
        BNode* node_to = event->NodeTo();
        
        std::vector<Event*> events_to_disable = hole_transfer_map.at(node_from);
        std::vector<Event*> events_to_enable = hole_transfer_map.at(node_to);

        event->AddDisableOnExecute(&events_to_disable);
        event->AddEnableOnExecute(&events_to_enable);
    }
    
    for (auto& event: et_events ) {
        BNode* node_from = event->NodeFrom();
        BNode* node_to = event->NodeTo();
        
        std::vector<Event*> events_to_disable = electron_transfer_map.at(node_from);
        std::vector<Event*> events_to_enable = electron_transfer_map.at(node_to);

        event->AddDisableOnExecute(&events_to_disable);
        event->AddEnableOnExecute(&events_to_enable);
    }
    
    // for every event associated with node_from - create a list of events to check after onExecute
    // Events to check are every event for each node_to
    
    //hole transfer events to check
    for (auto& event: ht_events ) {
        
        BNode* node_from = event->NodeFrom();
        std::vector<Event*> ht_events = hole_transfer_map.at(node_from);
        
        for (std::vector<Event*>::iterator it_event = ht_events.begin(); it_event != ht_events.end(); ++it_event) {          
            Event* event_move_hole = *it_event;
            HoleTransfer* hole_transfer = dynamic_cast<HoleTransfer*> (event_move_hole);
            
            BNode* node_to = hole_transfer->NodeTo();
            std::vector<Event*> events_to_check = hole_transfer_map.at(node_to);
            event->CheckEventsOnExecute(&events_to_check);
            
        }
    }
    
    //electron transfer check events 
    for (auto& event: et_events ) {
        
        BNode* node_from = event->NodeFrom();
        std::vector<Event*> et_events = electron_transfer_map.at(node_from);
        
        for (std::vector<Event*>::iterator it_event = et_events.begin(); it_event != et_events.end(); ++it_event) {
            Event* event_move_electron = *it_event;
            ElectronTransfer* electron_transfer = dynamic_cast<ElectronTransfer*> (event_move_electron);
            
            BNode* node_to = electron_transfer->NodeTo();
            std::vector<Event*> events_to_check = electron_transfer_map.at(node_to);
            
            event->CheckEventsOnExecute(&events_to_check);
        }
    }

    //for (auto& event: ct_events ) event->Print();
    
    // organize events in a tree;
    // first level VSSM events (escape event for each carrier))
    for (State::iterator carrier = _state->begin(); carrier != _state->end(); ++carrier) {
        
        //std::cout << "Adding escape event for carrier " << (*carrier)->Type() << ", id " << (*carrier)->id() << std::endl;
         
        BNode* node_from = (*carrier)->GetNode();
        
        Event* event_escape = Events().Create("carrier_escape");
        CarrierEscape* carrier_escape = dynamic_cast<CarrierEscape*> (event_escape);
        carrier_escape->Initialize((*carrier));
        head_event.AddSubordinate( event_escape );
 
        std::vector<Event*> ht_events = hole_transfer_map.at(node_from);        
        std::vector<Event*> et_events = electron_transfer_map.at(node_from);
        
        // Add move events from the map 
        for (std::vector<Event*>::iterator it_ht_event = ht_events.begin(); it_ht_event != ht_events.end(); ++it_ht_event) {

            // New event - hole transfer
            if ((*carrier)->Type() == "hole"){
                Event* event_move_hole = *it_ht_event;
                HoleTransfer* hole_transfer = dynamic_cast<HoleTransfer*> (event_move_hole);
                Hole* hole = dynamic_cast<Hole*> ((*carrier));
                hole_transfer->SetHole( hole );
                hole_transfer->Enable();
                carrier_escape->AddSubordinate( event_move_hole );  
            }
        }
        
        for (std::vector<Event*>::iterator it_et_event = et_events.begin(); it_et_event != et_events.end(); ++it_et_event) {
            
            // New event - electron transfer
            if ((*carrier)->Type() == "electron"){
                Event* event_move_electron = *it_et_event;
                ElectronTransfer* electron_transfer = dynamic_cast<ElectronTransfer*> (event_move_electron);
                Electron* electron = dynamic_cast<Electron*> ((*carrier));
                electron_transfer->SetElectron( electron );
                electron_transfer->Enable();
                carrier_escape->AddSubordinate( event_move_electron );  
            }
        }   
       
    }
    head_event.Enable();

}

void Run( double runtime, int nsteps, votca::tools::Random2 *RandomVariable, int nelectrons, int nholes, string trajectoryfile, double outtime, double fieldX, double fieldY, double fieldZ) {

    std::cout << std::endl << "Starting the KMC loop" << std::endl;
   
    clock_t begin = clock();
    
    double time = 0.0;
    int step = 0;
    double trajout = 0.0;
    //progressbar(0.);
    
    
    if ( runtime != 0 && nsteps == 0 ){ 
        //runtime = runtime;
        //progressbar(time/runtime);
        std::cout << "Specified runtime (s): " << runtime << std::endl; 
    }
    else if ( nsteps != 0 && runtime == 0 ){ 
        //nsteps = nsteps; 
        std::cout << "Specified number of simulation steps: " << nsteps << std::endl;
    }
    else if ( runtime !=0 && nsteps !=0 ){ 
        //nsteps = nsteps;
	runtime = 0.0; 
        std::cout << "Specified both the number of simulation steps and the runtime -> number of steps will be used. " << std::endl; 
        std::cout << "Specified number of simulation steps: " << nsteps << std::endl;
    }
    
    state->Trajectory_create(trajectoryfile);
    state->Trajectory_write(trajout, trajectoryfile);
    
    trajout = outtime;
    //progressbar(0.);
    
    while ( step < nsteps || time < runtime ){  
        
        //head_event.Print();
           
        double u = 1.0 - RandomVariable->rand_uniform();
        while(u == 0.0){ u = 1.0 - RandomVariable->rand_uniform();}
        double elapsed_time = -1.0 / head_event.CumulativeRate() * log(u);
        state->AdvanceClock(elapsed_time);
        time += elapsed_time;
        step++;
        //clock_t snap = clock();
        
        //execute the event after the time update - so the elapsed time is calculated with the correct rate (the rate before the event)
        head_event.OnExecute(state, RandomVariable );
        //std::cout << "Time: " << time << std::endl;
        
        //include if the progress bar is required 
        /*if ( runtime != 0 && nsteps == 0 )
        {
            progressbar(time/runtime);
            //printf("Remaining time: %f seconds ", ((double)(snap - begin)*(int(runtime/(time-1))) / CLOCKS_PER_SEC));
        }
        else
        {
            //progressbar(double(step)/double(nsteps));
            //std::cout << "   remaining: " << (double (snap-begin))*(double(nsteps)/double(step)-1);
            //printf("Remaining time: %f seconds ---  %i steps complete", ((double)(snap - begin)*nsteps/(step-1)) / CLOCKS_PER_SEC, step);
           
        }*/
    
        
        //write to the trajectory file at every outtime (specified in the options file)
        if (outtime != 0 && trajout < time )
        {
            state->Trajectory_write(trajout, trajectoryfile);
            trajout = time + outtime;
        }

    } 
        
    //progressbar(1.);
    state->Print_properties(nelectrons, nholes, fieldX, fieldY, fieldZ);
    clock_t end = clock();    
    printf("Elapsed: %f seconds after %i steps \n", (double)(end - begin) / CLOCKS_PER_SEC, step);
     
}

private:
    
    // head event, contains all escape events
    CarrierEscape head_event;
   
};
    
}}



#endif

