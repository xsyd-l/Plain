#include "include/spark/Spark.hpp"
#include <queue>
#include <deque>
#include <iostream>

SparkSoA::SparkSoA(std::filesystem::path font_path) {
    if (!font_.openFromFile(font_path)) {
        std::cout<<"error:connot load font";
        exit(0);
    }
}

void SparkSoA::spawnSpark(SparkType type, std::string content) {
    switch(type) {
        case SparkType::damage:{
            
            break;
        }
        case SparkType::heal:{
            break;
        }
        default:{
            break;
        }
    }
}

void SparkSoA::updateSpark() {
    // 遍历deque
    for (auto& spark: active_deque_) {
        if (spark.life > 0) {
            if (spark.life > spark.point) {
                spark.life--;
                //速度积分
                spark.velocity += spark.acceleration;
                //位移积分
                spark.text.move(spark.velocity);
            }
            else if (spark.zoom > 0) {
                spark.zoom -= (float)1/ spark.point;
            }
        }
    }
}