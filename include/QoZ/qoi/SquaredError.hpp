//
// Created by Xin Liang on 12/07/2021.
//

#ifndef SZ_QOI_SQUARED_ERROR_HPP
#define SZ_QOI_SQUARED_ERROR_HPP

#include "QoZ/def.hpp"
#include "QoZ/qoi/QoI.hpp"
#include "QoZ/utils/Iterator.hpp"

namespace QoZ {
    template<class T, uint N>
    class QoI_SquaredError : public concepts::QoIInterface<T, N> {

    public:
        QoI_SquaredError(double tolerance, size_t num_elements, T global_eb) : 
                tolerance(tolerance),
                num_rest(num_elements),
                global_eb(global_eb) {
            printf("tolerance = %.4e\n", (double) tolerance);
            printf("global_eb = %.4e\n", (double) global_eb);
            block_eb = compute_eb();
        }

        using Range = multi_dimensional_range<T, N>;
        using iterator = typename multi_dimensional_range<T, N>::iterator;

        T interpret_eb(T data) const {
            return std::min(block_eb, global_eb);
        }

        T interpret_eb(const iterator &iter) const {
            // assume uniform distribution
            // printf("eb = %.4f\n", sqrt(3*tolerance / num_rest));
            // exit(0);
            return std::min(block_eb, global_eb);
        }

        T interpret_eb(const T * data, ptrdiff_t offset) const {
            return interpret_eb(*data);
        }

        void update_tolerance(T data, T dec_data){
            tolerance = tolerance - (data - dec_data) * (data - dec_data);
            num_rest --;
        }

        void precompress_block(const std::shared_ptr<Range> &range){}

        void postcompress_block(){
            block_eb = compute_eb();
        }

        void print(){}

        double eval(T val) const{
            
            return 0;//todo

        } 

        std::string get_expression(const std::string var="x") const{
            return "";
        } 

        void pre_compute(const T * data){}

        void set_qoi_tolerance(double tol) {tolerance = tol;}

    private:
        T compute_eb(){
            return sqrt(3 * tolerance / num_rest);
        }
        double tolerance;
        T global_eb;
        int num_rest;
        T block_eb = 0;
    };
}
#endif 