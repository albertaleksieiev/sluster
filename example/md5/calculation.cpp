//
// Created by Albert on 6/6/17.
//
#include "enumerations.h"
#include "../../sluster.h"
#include "md5.h"

#include <unordered_map>

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <boost/foreach.hpp>

#include <chrono>
#include <thread>

char buf[16394];

std::unordered_set<std::string> md5Passwords;
std::vector<std::string> crackedPasswords;
long long curr_progress = 0;
long long max_progress = 0;
long long keyLenght = 0;
auto enumerator = Enumerations::Enumerator();



template <typename T>
std::vector<T> as_vector(boost::property_tree::ptree const& pt, boost::property_tree::ptree::key_type const& key)
{
    std::vector<T> r;
    for (auto& item : pt.get_child(key))
        r.push_back(item.second.get_value<T>());
    return r;
}
bool start = false;
void on_message(int sockfd, char* msg, struct sockaddr *their_addr, socklen_t addr_len){


    using boost::property_tree::ptree;
    using boost::property_tree::read_json;
    using boost::property_tree::write_json;

    try {
        ptree messageObj;
        std::istringstream is(msg);
        read_json(is, messageObj);


        std::string command = messageObj.get<std::string>("command");
        if (command == "add") {
            for (std::string md5: as_vector<std::string>(messageObj, "passwords")) {
                boost::algorithm::to_lower(md5);
                std::cout<<"add pass : "<<md5<<std::endl;
                md5Passwords.insert(md5);
            }
            sprintf(buf, "added %d passwords", (int) md5Passwords.size());
        } else if (command == "start-bruteforce") {

            max_progress = 1;
            BOOST_FOREACH(const ptree::value_type &child,
                          messageObj.get_child("ranges")) {
                            char begin = child.second.get<char>("begin");

                            char end = child.second.get<char>("end");

                            enumerator.addEnumeration(std::to_string(keyLenght),
                                                      Enumerations::Enumeration<char>(begin,end, 1));

                            max_progress *= (end - begin + 1);
                            keyLenght++;
                        }

            sprintf(buf, "starting, total count : %lld", max_progress);
            start = true;
        } else if (command == "status") {
            ptree pt;
            pt.put("current-progress", curr_progress);
            pt.put("max-progress", max_progress);
            ptree cracked;
            for (auto &crackedPass: crackedPasswords) {
                ptree pass;
                pass.put("pass", crackedPass);
                pass.put("md5", md5(crackedPass));
                cracked.push_back(std::make_pair("", pass));
            }

            pt.add_child("cracked-passwords", cracked);

            std::ostringstream osbuf;
            write_json(osbuf, pt, false);

            strcpy(buf, osbuf.str().c_str());
        }


        sendto(sockfd, buf, strlen(buf), 0, their_addr, addr_len);
    }catch (std::exception e){

    }
}

void calcFunc(void*){

    while(!start){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }


    std::string pass(keyLenght,' ');
    enumerator.enumerate([&pass](Enumerations::EnumerationMap enumerationMap){

        for(int i=0;i<keyLenght;i++){
            pass[i] = enumerationMap.get<char>(std::to_string(i));
        }
        //std::cout<<pass<<" -> "<<md5(pass)<<std::endl;

        if(md5Passwords.find(md5(pass)) != md5Passwords.end()){
            std::cout<<"Finded password : "<<pass<<std::endl;
            crackedPasswords.push_back(pass);
            md5Passwords.erase(md5(pass));
        }
        curr_progress++;

    });
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    }
}

int main(){

    CalculationSocketManager calculationSocketManager("4950",on_message);

    calculationSocketManager.start(calcFunc);

}

