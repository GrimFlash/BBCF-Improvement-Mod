#include "ScrWindow.h"

#include "Core/interfaces.h"
#include "Core/Settings.h"
#include "Core/utils.h"
#include "Game/gamestates.h"
#include "Overlay/NotificationBar/NotificationBar.h"
#include "Overlay/WindowManager.h"
#include "Overlay/Window/HitboxOverlay.h"

#include "Psapi.h"
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <windows.h>

void ScrWindow::Draw()
{
    if (m_showDemoWindow)
    {
        ImGui::ShowDemoWindow(&m_showDemoWindow);
    }

    DrawStatesSection();
    DrawPlaybackSection();
}

void ScrWindow::DrawStatesSection()
{
    
    if (!ImGui::CollapsingHeader("States"))
        return;
    if (*g_gameVals.pGameMode != GameMode_Training) {
        ImGui::Text("Only works in training mode");
        return;
    }
    if (g_interfaces.player2.IsCharDataNullPtr()  || g_interfaces.player2.GetData()->charIndex == g_interfaces.player1.GetData()->charIndex) {
        ImGui::TextWrapped("Something invalid, you are in training mode char select, have 2 of the same characters or some other shit i haven't figured out yet that you should tell me so i can fix");
        return;
    }
    static int selected = 0;
    //Code for auto loading script upon character switch, prob move it to OnMatchInit() or smth
   if (p2_old_char_data == NULL || p2_old_char_data != (void*)g_interfaces.player2.GetData()){
        char* bbcf_base_adress = GetBbcfBaseAdress();
        std::vector<scrState*> states = parse_scr(bbcf_base_adress, 2);
        g_interfaces.player2.SetScrStates(states);
        g_interfaces.player2.states = states;
        p2_old_char_data = (void*)g_interfaces.player2.GetData();
        gap_register = {};
        wakeup_register = {};
        selected = 0;
    }


    if (ImGui::Button("Force Load P2 Script")) {
        char* bbcf_base_adress = GetBbcfBaseAdress();
        std::vector<scrState*> states = parse_scr(bbcf_base_adress, 2);
        g_interfaces.player2.SetScrStates(states);
        g_interfaces.player2.states = states;
        gap_register = {};
        wakeup_register = {};
        selected = 0;
    }
    auto states = g_interfaces.player2.states;
    {
        ImGui::BeginChild("left pane", ImVec2(200, 0), true);
        for (int i = 0; i < g_interfaces.player2.states.size(); i++)
        {
            std::string label= g_interfaces.player2.states[i]->name;
            if (ImGui::Selectable(label.c_str(), selected == i))
                selected = i;
        }
        ImGui::EndChild();
        ImGui::SameLine();
    }
    // Right
    {
        ImGui::BeginGroup();
        static bool isActive_old;
        static bool isActive = false;
        if (ImGui::Checkbox("Naoto EN specials toggle", &isActive)) {
               memset(&g_interfaces.player2.GetData()->slot2_or_slot4, 0x00000018, 4);
        }
        if (isActive) {
            memset(&g_interfaces.player2.GetData()->slot2_or_slot4, 0x00000018, 4);
        }
        else {
            if (isActive != isActive_old) {
                memset(&g_interfaces.player2.GetData()->slot2_or_slot4, 0, 4);
            }
        }
        isActive_old = isActive;
        //ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 100)); // Leave room for 1 line below us
        ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 150)); // Leave room for 1 line below us
        if (states.size()>0){
            auto selected_state = states[selected];
            ImGui::Text("%s", selected_state->name.c_str());
            ImGui::Separator();
            ImGui::Text("Addr: 0x%x", selected_state->addr);
            ImGui::Text("Frames: %d", selected_state->frames);
            ImGui::Text("Damage: %d",  selected_state->damage);
            ImGui::Text("Atk_level: %d", selected_state->atk_level);
            ImGui::Text("Hitstun: %d", selected_state->hitstun);
            ImGui::Text("Blockstun: %d", selected_state->blockstun);
            ImGui::Text("Hitstop: %d", selected_state->hitstop);
            ImGui::Text("Starter_rating: %d", selected_state->starter_rating);
            ImGui::Text("Atk_P1: %d", selected_state->attack_p1);
            ImGui::Text("Atk_P2: %d", selected_state->attack_p2);
            ImGui::Text("Hit_overhead: %d", selected_state->hit_overhead);
            ImGui::Text("Hit_low: %d", selected_state->hit_low);
            ImGui::Text("Hit_air_ublockable: %d", selected_state->hit_air_unblockable);
            ImGui::Text("fatal_counter: %d", selected_state->fatal_counter);
            ImGui::Text("Whiff_cancels:", selected_state->fatal_counter);
            for (std::string name: selected_state->whiff_cancel) {
                ImGui::Text("    %s", name.c_str());
            }
            ImGui::Text("Hit_or_block_cancels(gatlings):", selected_state->fatal_counter);
            int item_view_len;
            if (selected_state->hit_or_block_cancel.size() > 5){
               item_view_len = 100; }
            else {
               item_view_len = selected_state->hit_or_block_cancel.size() * 20;
            }
            ImGui::BeginChild("item view", ImVec2(0, item_view_len));
            for (std::string name : selected_state->hit_or_block_cancel) {
                ImGui::Text("    %s", name.c_str());
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();


        
        ImGui::Separator();
        if (ImGui::Button("Set as wakeup action")) {
            states = g_interfaces.player2.states;
            auto selected_state = states[selected];
            std::string substr = "CmnActUkemiLandNLanding";
            std::vector<std::string> matches = {};
            for (auto state : states) {
                std::string name = state->name;
                if (name.find(substr) != std::string::npos) {
                    if (!state->replaced_state_script[0]) {
                        memcpy(state->replaced_state_script, state->addr + 36, 36);
                    }
                    override_state(state->addr, &selected_state->name[0]);

                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Set as gap action")) {
            states = g_interfaces.player2.states;
            auto selected_state = states[selected];
            std::string substr = "GuardEnd";
            std::vector<std::string> matches = {};
            for (auto state : states) {
                std::string name = state->name;
                if (name.find(substr) != std::string::npos) {
                    if (!state->replaced_state_script[0]) {
                        memcpy(state->replaced_state_script, state->addr + 36, 36);
                    }
                    override_state(state->addr, &selected_state->name[0]);

                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Use")) {
            states = g_interfaces.player2.states;
            auto selected_state = states[selected];
            memcpy(&(g_interfaces.player2.GetData()->currentScriptActionLocationInMemory), &(selected_state->addr),4);
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
            states = g_interfaces.player2.states;
            for (auto state : states) {
                if (state->replaced_state_script[0]) {
                    memcpy(state->addr + 36, state->replaced_state_script, 36);
                    state->replaced_state_script[0] = 0;
                
                }
            }
            gap_register = {};
            wakeup_register = {};
        }
        
    
        if (ImGui::CollapsingHeader("Gap/wakeup random actions")) {
            ImGui::Columns(2);
            if (ImGui::Button("Add to wakeup action")) {
                states = g_interfaces.player2.states;
                wakeup_register.push_back(states[selected]);
            }
            ImGui::BeginChild("wakeup_register_display");
            for (auto e : wakeup_register) {
                ImGui::Text(e->name.c_str());
            }
            ImGui::EndChild();
            ImGui::NextColumn();
            if (ImGui::Button("Add to gap action")) {
                states = g_interfaces.player2.states;
                gap_register.push_back(states[selected]);
            }
        
            ImGui::BeginChild("gap_register_display");
            for (auto e : gap_register) {
                ImGui::Text(e->name.c_str());
            }
            ImGui::EndChild();
            if (!wakeup_register.empty()) {
                states = g_interfaces.player2.states;
                auto selected_state = states[selected];
                int random_pos = std::rand() % wakeup_register.size();
                std::string substr = "CmnActUkemiLandNLanding";
                for (auto state : states) {
                    std::string name = state->name;
                    if (name.find(substr) != std::string::npos) {
                        if (!state->replaced_state_script[0]) {
                            memcpy(state->replaced_state_script, state->addr + 36, 36);
                        }
                        override_state(state->addr, &wakeup_register[random_pos]->name[0]);

                    }
                }
            }


            if (!gap_register.empty()) {
                states = g_interfaces.player2.states;
                auto selected_state = states[selected];
                int random_pos = std::rand() % gap_register.size();
                std::string substr = "GuardEnd";
                for (auto state : states) {
                    std::string name = state->name;
                    if (name.find(substr) != std::string::npos) {
                        if (!state->replaced_state_script[0]) {
                            memcpy(state->replaced_state_script, state->addr + 36, 36);
                        }
                        override_state(state->addr, &gap_register[random_pos]->name[0]);
                    }
                }
            }
        }
        //ImGui::EndChild();
        ImGui::EndGroup(); 
    
    }

}
std::string interpret_move(char move) {
    //auto button_bits = move & ((4 << 1) - 1);
    auto button_bits = move & 0xf0;
    auto direction_bits = move & 0x0f;
    //auto direction_bits = move & ((8 << 1) - 1);
    std::string move_t{ "" };
    switch (direction_bits) {
    case 0x5:
        move_t += "Neutral";
        break;
    case 0x4:
        move_t += "Left";
        break;
    case 0x1:
        move_t += "DownLeft";
        break;
    case 0x2:
        move_t += "Down";
        break;
    case 0x3:
        move_t += "DownRight";
        break;
    case 0x6:
        move_t += "Right";
        break;
    case 0x9:
        move_t += "UpRight";
        break;
    case 0x8:
        move_t += "Up";
        break;
    case 0x7:
        move_t += "UpLeft";
        break;
    }
    switch (button_bits) {
    case 0x10:
        move_t += "+A";
        break;
    case 0x20:
        move_t += "+B";
        break;
    case 0x40:
        move_t += "+C";
        break;
    case 0x80:
        move_t += "+D";
        break;
    case 0x30:
        move_t += "+A+B";
        break;
    case 0x50:
        move_t += "+A+C";
        break;
    case 0x90:
        move_t += "+A+D";
        break;
    case 0x60:
        move_t += "+B+C";
        break;
    case 0xA0:
        move_t += "+B+D";
        break;
    case 0xC0:
        move_t += "+C+D";
        break;
    case 0xB0:
        move_t += "+A+B+D";
        break;
    case 0x70:
        move_t += "+A+B+D";
        break;
    case 0xD0:
        move_t += "+A+C+D";
        break;
    case 0xE0:
        move_t += "+B+C+D";
        break;
    case 0xF0:
        move_t += "+A+B+C+D";
        break;
    }

    return move_t;
}
void save_to_file(std::vector<char> slot_buffer, char facing_direction, char* fname) {
    CreateDirectory(L"slots", NULL);
    std::string fpath = "./slots/";
    fpath += fname;
    std::ofstream out(fpath);
    out << facing_direction;
    for (const auto& e : slot_buffer) out << e;
    out.close();
}
std::vector<char> load_from_file(char* fname) {
    std::string fpath = "./slots/";
    fpath +=fname;
    //char* filename = "./slots/" + fpath;
    std::streampos fileSize;
    std::ifstream file(fpath, std::ios::binary);
    if (file.good()) {
        file.seekg(0, std::ios::end);
        fileSize = file.tellg();
        file.seekg(0, std::ios::beg);


        std::vector<char> fileData(fileSize);
        file.read((char*)&fileData[0], fileSize);
        return fileData;
    }

    else {
        std::vector<char> fd{};
        return fd;
    }
 

}
std::vector<char> trim_playback(std::vector<char> slot_buffer) {
    int i = 0;
    //trim the start
    while (i < slot_buffer.size() && slot_buffer[0] == 5) {
        slot_buffer.erase(slot_buffer.begin());
        i++;
    }
    return slot_buffer;
}
void load_trimmed_playback(std::vector<char> trimmed_playback,char* frame_len_slot_p, char* start_of_slot_inputs) {
    //trim the start
    //auto trimmed_playback = trim_playback(slot_buffer);
    int frame_len_loaded_file = trimmed_playback.size();
    memcpy(frame_len_slot_p, &(frame_len_loaded_file), 4);
    int iter = 0;
    for (auto input : trimmed_playback) {
        memcpy(start_of_slot_inputs + (iter * 2), &input, 2);
        iter++;
    }
    }

void ScrWindow::DrawPlaybackSection() {
    char* bbcf_base_adress = GetBbcfBaseAdress();
    if (ImGui::CollapsingHeader("Playback")) {

        if (ImGui::CollapsingHeader("SLOT_1")) {




            int time_count_slot_1_addr_offset = 0x9075E8;
            char* frame_len_slot_p = bbcf_base_adress + 0x9075E8;
            int frame_len_slot;
            memcpy(&frame_len_slot, frame_len_slot_p, 4);


            int facing_direction_slot_1_addr_offset = 0x9075D8;
            char* facing_direction_p = bbcf_base_adress + facing_direction_slot_1_addr_offset;
            int facing_direction;
            memcpy(&facing_direction, facing_direction_p, 4);



            char* start_of_slot_inputs = bbcf_base_adress + time_count_slot_1_addr_offset + 0x10;
            std::vector<char> slot1_recording_frames{};
            for (int i = 0; i < frame_len_slot; i++) {
                slot1_recording_frames.push_back(*(start_of_slot_inputs + i * 2));
            }
            if (ImGui::Button("Save##slot1")) {
                save_to_file(slot1_recording_frames, facing_direction, fpath_s1);
            }
            ImGui::SameLine();
            if (ImGui::Button("Load##slot1")) {
                auto loaded_file = load_from_file(fpath_s1);
                if (!loaded_file.empty()) {
                    char facing_direction = loaded_file[0];
                    loaded_file.erase(loaded_file.begin());
                    memcpy(facing_direction_p, &(facing_direction), sizeof(char));
                }
                int frame_len_loaded_file = loaded_file.size();
                memcpy(frame_len_slot_p, &(frame_len_loaded_file), 4);
                int iter = 0;

                if (!loaded_file.empty()) {
                    for (auto input : loaded_file) {
                        memcpy(start_of_slot_inputs + (iter * 2), &input, 2);
                        iter++;
                    }
                }


                std::cout << "oi" << std::endl;
            }
            ImGui::SameLine();
            if (ImGui::Button("Trim Playback##slot1")) {
                slot1_recording_frames = trim_playback(slot1_recording_frames);
                load_trimmed_playback(slot1_recording_frames, frame_len_slot_p, start_of_slot_inputs);
            }
            ImGui::InputText("File Path##slot1", fpath_s1, IM_ARRAYSIZE(fpath_s1));
            ImGui::TextWrapped("If the field isn't accepting keyboard input, try alt-tabbing out and back in, if that doesn't work copy and paste should still work(or restarting the game)");
            ImGui::Separator();
            auto old_val = 0; auto frame_counter = 0;
            for (auto el : slot1_recording_frames) {
                frame_counter++;
                if (old_val != el) {
                    std::string move_string = interpret_move(el);
                    ImGui::Text("frame %d: %s (0x%x)", frame_counter, move_string.c_str(), el);
                    old_val = el;
                }
            }
        }

        if (ImGui::CollapsingHeader("SLOT_2")) {




            int time_count_slot_2_addr_offset = 0x9075EC;
            char* frame_len_slot_p = bbcf_base_adress + time_count_slot_2_addr_offset;
            int frame_len_slot;
            memcpy(&frame_len_slot, frame_len_slot_p, 4);


            int facing_direction_slot_2_addr_offset = 0x9075DC;
            char* facing_direction_p = bbcf_base_adress + facing_direction_slot_2_addr_offset;
            int facing_direction;
            memcpy(&facing_direction, facing_direction_p, 4);


            //0x960 is 2400 which is the size of the recording slot_1, recording slot_2 has this offset + 0x10 relative to start of slot_1
            char* start_of_slot_inputs = bbcf_base_adress + time_count_slot_2_addr_offset + 0x960+ 0x10;
            std::vector<char> slot2_recording_frames{};
            for (int i = 0; i < frame_len_slot; i++) {
                slot2_recording_frames.push_back(*(start_of_slot_inputs + i * 2));
            }
            if (ImGui::Button("Save##slot2")) {
                save_to_file(slot2_recording_frames, facing_direction, fpath_s2);
            }
            ImGui::SameLine();
            if (ImGui::Button("Load##slot2")) {
                auto loaded_file = load_from_file(fpath_s2);
                if (!loaded_file.empty()) {
                    char facing_direction = loaded_file[0];
                    loaded_file.erase(loaded_file.begin());
                    memcpy(facing_direction_p, &(facing_direction), sizeof(char));
                }
                int frame_len_loaded_file = loaded_file.size();
                memcpy(frame_len_slot_p, &(frame_len_loaded_file), 4);
                int iter = 0;

                if (!loaded_file.empty()) {
                    for (auto input : loaded_file) {
                        memcpy(start_of_slot_inputs + (iter * 2), &input, 2);
                        iter++;
                    }
                }


                std::cout << "oi" << std::endl;
            }
            ImGui::SameLine();
            if (ImGui::Button("Trim Playback##slot2")) {
                slot2_recording_frames = trim_playback(slot2_recording_frames);
                load_trimmed_playback(slot2_recording_frames, frame_len_slot_p, start_of_slot_inputs);
            }
            ImGui::InputText("File Path##slot2", fpath_s2, IM_ARRAYSIZE(fpath_s2));
            ImGui::TextWrapped("If the field isn't accepting keyboard input, try alt-tabbing out and back in, if that doesn't work copy and paste should still work(or restarting the game)");
            ImGui::Separator();
            auto old_val = 0; auto frame_counter = 0;
            for (auto el : slot2_recording_frames) {
                frame_counter++;
                if (old_val != el) {
                    std::string move_string = interpret_move(el);
                    ImGui::Text("frame %d: %s (0x%x)", frame_counter, move_string.c_str(), el);
                    old_val = el;
                }
            }

        }
        
        if (ImGui::CollapsingHeader("SLOT_3")) {




            int time_count_slot_3_addr_offset = 0x9075F0;
            char* frame_len_slot_p = bbcf_base_adress + time_count_slot_3_addr_offset;
            int frame_len_slot;
            memcpy(&frame_len_slot, frame_len_slot_p, 4);


            int facing_direction_slot_3_addr_offset = 0x9075E0;
            char* facing_direction_p = bbcf_base_adress + facing_direction_slot_3_addr_offset;
            int facing_direction;
            memcpy(&facing_direction, facing_direction_p, 4);


            //similar to slot_2
            char* start_of_slot_inputs = bbcf_base_adress + time_count_slot_3_addr_offset + (0x960 * 2) + 0x10;
            std::vector<char> slot3_recording_frames{};
            for (int i = 0; i < frame_len_slot; i++) {
                slot3_recording_frames.push_back(*(start_of_slot_inputs + i * 2));
            }
            if (ImGui::Button("Save##slot3")) {
                save_to_file(slot3_recording_frames, facing_direction, fpath_s3);
            }
            ImGui::SameLine();
            if (ImGui::Button("Load##slot3")) {
                auto loaded_file = load_from_file(fpath_s3);
                if (!loaded_file.empty()) {
                    char facing_direction = loaded_file[0];
                    loaded_file.erase(loaded_file.begin());
                    memcpy(facing_direction_p, &(facing_direction), sizeof(char));
                }
                int frame_len_loaded_file = loaded_file.size();
                memcpy(frame_len_slot_p, &(frame_len_loaded_file), 4);
                int iter = 0;

                if (!loaded_file.empty()) {
                    for (auto input : loaded_file) {
                        memcpy(start_of_slot_inputs + (iter * 2), &input, 2);
                        iter++;
                    }
                }


                std::cout << "oi" << std::endl;
            }
            ImGui::SameLine();
            if (ImGui::Button("Trim Playback##slot3")) {
                slot3_recording_frames = trim_playback(slot3_recording_frames);
                load_trimmed_playback(slot3_recording_frames, frame_len_slot_p, start_of_slot_inputs);
            }
            ImGui::InputText("File Path##slot3", fpath_s3, IM_ARRAYSIZE(fpath_s3));
            ImGui::TextWrapped("If the field isn't accepting keyboard input, try alt-tabbing out and back in, if that doesn't work copy and paste should still work(or restarting the game)");
            ImGui::Separator();
            auto old_val = 0; auto frame_counter = 0;
            for (auto el : slot3_recording_frames) {
                frame_counter++;
                if (old_val != el) {
                    std::string move_string = interpret_move(el);
                    ImGui::Text("frame %d: %s (0x%x)", frame_counter, move_string.c_str(), el);
                    old_val = el;
                }
            }

        }

        if (ImGui::CollapsingHeader("SLOT_4")) {




            int time_count_slot_4_addr_offset = 0x9075F4;
            char* frame_len_slot_p = bbcf_base_adress + time_count_slot_4_addr_offset;
            int frame_len_slot;
            memcpy(&frame_len_slot, frame_len_slot_p, 4);


            int facing_direction_slot_4_addr_offset = 0x9075E4;
            char* facing_direction_p = bbcf_base_adress + facing_direction_slot_4_addr_offset;
            int facing_direction;
            memcpy(&facing_direction, facing_direction_p, 4);


            //similar to slot_2 and slot_3
            char* start_of_slot_inputs = bbcf_base_adress + time_count_slot_4_addr_offset + (0x960 * 3) + 0x10;
            std::vector<char> slot4_recording_frames{};
            for (int i = 0; i < frame_len_slot; i++) {
                slot4_recording_frames.push_back(*(start_of_slot_inputs + i * 2));
            }
            if (ImGui::Button("Save##slot4")) {
                save_to_file(slot4_recording_frames, facing_direction, fpath_s4);
            }
            ImGui::SameLine();
            if (ImGui::Button("Load##slot4")) {
                auto loaded_file = load_from_file(fpath_s4);
                if (!loaded_file.empty()) {
                    char facing_direction = loaded_file[0];
                    loaded_file.erase(loaded_file.begin());
                    memcpy(facing_direction_p, &(facing_direction), sizeof(char));
                }
                int frame_len_loaded_file = loaded_file.size();
                memcpy(frame_len_slot_p, &(frame_len_loaded_file), 4);
                int iter = 0;

                if (!loaded_file.empty()) {
                    for (auto input : loaded_file) {
                        memcpy(start_of_slot_inputs + (iter * 2), &input, 2);
                        iter++;
                    }
                }


                std::cout << "oi" << std::endl;
            }
            ImGui::SameLine();
            if (ImGui::Button("Trim Playback##slot4")) {
                slot4_recording_frames = trim_playback(slot4_recording_frames);
                load_trimmed_playback(slot4_recording_frames, frame_len_slot_p, start_of_slot_inputs);
            }
            ImGui::InputText("File Path##slot4", fpath_s4, IM_ARRAYSIZE(fpath_s4));
            ImGui::TextWrapped("If the field isn't accepting keyboard input, try alt-tabbing out and back in, if that doesn't work copy and paste should still work(or restarting the game)");
            ImGui::Separator();
            auto old_val = 0; auto frame_counter = 0;
            for (auto el : slot4_recording_frames) {
                frame_counter++;
                if (old_val != el) {
                    std::string move_string = interpret_move(el);
                    ImGui::Text("frame %d: %s (0x%x)", frame_counter, move_string.c_str(), el);
                    old_val = el;
                }
            }

        }
}
}