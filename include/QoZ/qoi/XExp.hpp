//
// Created by Xin Liang on 12/06/2021.
//

#ifndef SZ_QOI_X_EXP_HPP
#define SZ_QOI_X_EXP_HPP

#include <algorithm>
#include "QoZ/def.hpp"
#include "QoZ/qoi/QoI.hpp"
#include "QoZ/utils/Iterator.hpp"

namespace QoZ {
    template<class T, uint N>
    class QoI_X_Exp : public concepts::QoIInterface<T, N> {

    public:
        QoI_X_Exp(T tolerance, T global_eb, double base = 2.0) : 
                tolerance(tolerance),
                global_eb(global_eb), base(base) {
            // TODO: adjust type for int data
            //printf("global_eb = %.4f\n", (double) global_eb);
            concepts::QoIInterface<T, N>::id = 12;
            log_base = log(base);
        }
        
        using Range = multi_dimensional_range<T, N>;
        using iterator = typename multi_dimensional_range<T, N>::iterator;

        T interpret_eb(T data) const {
            //base^X
            //double low_bound = data - log( pow(2, data) - tolerance)/ log(2);
            //double high_bound = log( pow(2, data) + tolerance) / log(2)-data;
            //T eb = std::min(low_bound,high_bound);
            double a = fabs(pow(base,data)*log_base );//datatype may be T
            double b = fabs(a*log_base);
            T eb = (sqrt(a*a+2*b*tolerance)-a)/b;
            return std::min(eb, global_eb);
            //return global_eb;
        }

        T interpret_eb(const iterator &iter) const {
            return interpret_eb(*iter);
        }

        T interpret_eb(const T * data, ptrdiff_t offset) {
            return interpret_eb(*data);
        }

        bool check_compliance(T data, T dec_data, bool verbose=false) const {
            return (fabs(pow(base,data) - pow(base,dec_data)) < tolerance);
        }

        void update_tolerance(T data, T dec_data){}

        void precompress_block(const std::shared_ptr<Range> &range){}

        void postcompress_block(){}

        void print(){}

        T get_global_eb() const { return global_eb; }

        void set_global_eb(T eb) {global_eb = eb;}

        void init(){}

        void set_dims(const std::vector<size_t>& new_dims){}

        double eval(T val) const{
            
            return pow(base,val);//todo

        } 

    private:
        T tolerance;
        T global_eb;
        double base;
        double log_base;
    };
}
#endif 
