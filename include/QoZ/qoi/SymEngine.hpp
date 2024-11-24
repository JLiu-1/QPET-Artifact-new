#ifndef SZ_QOI_SE_HPP
#define SZ_QOI_SE_HPP

#include <algorithm>
#include <cmath>
#include <functional>
#include <symengine/expression.h>
#include <symengine/parser.h>
#include <symengine/symbol.h>
#include <symengine/derivative.h>
#include <symengine/eval.h> 
#include <symengine/solve.h>
#include <symengine/functions.h>
#include <set>

using SymEngine::Expression;
using SymEngine::Symbol;
using SymEngine::symbol;
using SymEngine::parse;
using SymEngine::diff;
using SymEngine::Constant;
using SymEngine::RealDouble;
using SymEngine::Integer;
using SymEngine::evalf;
using SymEngine::map_basic_basic;
using SymEngine::down_cast;
using SymEngine::RCP;
using SymEngine::Basic;
using SymEngine::real_double;
using SymEngine::eval_double;

using SymEngine::E;
using SymEngine::eq;
using SymEngine::solve;
using SymEngine::rcp_static_cast;
using SymEngine::Mul;
using SymEngine::Pow;
using SymEngine::Log;
using SymEngine::sqrt;
using SymEngine::is_a;
using SymEngine::FiniteSet;

std::function<double(double, double, double)> convert_expression_to_function(const Basic &expr, 
                                                              const RCP<const Symbol> &x, 
                                                              const RCP<const Symbol> &y, 
                                                              const RCP<const Symbol> &z) {
            
        if (is_a<const SymEngine::Symbol>(expr) && expr.__eq__(*x)) {
            return [](double x_value, double, double) { /*std::cout<<"x="<<x_value<<std::endl;*/return x_value; };
        }
      
        else if (is_a<const SymEngine::Symbol>(expr) && expr.__eq__(*y)) {
            return [](double, double y_value, double) { /*std::cout<<"y="<<y_value<<std::endl;*/return y_value; };
        }
       
        else if (is_a<const SymEngine::Symbol>(expr) && expr.__eq__(*z)) {
            return [](double, double, double z_value) { /*std::cout<<"z="<<z_value<<std::endl;*/return z_value; };
        }
      
        else if (is_a<const RealDouble>(expr) or SymEngine::is_a<const Integer>(expr)) {
            double constant_value = eval_double(expr);
            return [constant_value](double, double, double) { /*std::cout<<"c="<<constant_value<<std::endl;*/return constant_value; };
        }

        // E
        else if ( eq(expr,*E)) {
            double e = std::exp(1);
            return [e](double, double, double) { return e; };
        }

       
        else if (is_a<SymEngine::Add>(expr)) {
            auto args = expr.get_args();
          
            std::vector<std::function<double(double, double, double)> > fs;
            for (size_t i = 0; i < args.size(); ++i) {

                fs.push_back(convert_expression_to_function(Expression(args[i]), x, y, z));
            }

           // auto first = convert_expression_to_function(Expression(args[0]), x, y, z);

            return [fs](double x_value, double y_value, double z_value) {
                double result = 0;
                for (auto &fnc:fs) {
                    result += fnc(x_value, y_value, z_value);
                }
                return result;
            };
        }
        else if (is_a<SymEngine::Mul>(expr)) {
            auto args = expr.get_args();
           
            std::vector<std::function<double(double, double, double)> > fs;
            for (size_t i = 0; i < args.size(); ++i) 
                fs.push_back(convert_expression_to_function(Expression(args[i]), x, y, z));

            return [ fs](double x_value, double y_value, double z_value) {
                double result = 1.0;
                for (auto &fnc:fs) {
                    result *= fnc(x_value, y_value, z_value);
                }
                return result;
            };
        }
       
        else if (is_a<SymEngine::Pow>(expr)) {
            auto args = expr.get_args();
            auto base = convert_expression_to_function(Expression(args[0]), x, y, z);
            auto exponent = convert_expression_to_function(Expression(args[1]), x, y, z);
            return [base, exponent](double x_value, double y_value, double z_value) {
                //std::cout<<"pow"<<std::endl;
                return std::pow(base(x_value, y_value, z_value), exponent(x_value, y_value, z_value));
            };
        }
      
        else if (is_a<SymEngine::Sin>(expr)) {
            auto arg = convert_expression_to_function(Expression(expr.get_args()[0]), x, y, z);
            return [arg](double x_value, double y_value, double z_value) {
                return std::sin(arg(x_value, y_value, z_value));
            };
        }
        
        else if (is_a<SymEngine::Cos>(expr)) {
            auto arg = convert_expression_to_function(Expression(expr.get_args()[0]), x, y, z);
            return [arg](double x_value, double y_value, double z_value) {
                return std::cos(arg(x_value, y_value, z_value));
            };
        }
        
        else if (is_a<SymEngine::Log>(expr)) {
            auto args = expr.get_args();
            auto arg = convert_expression_to_function(Expression(args[0]), x, y, z);
            if (args.size() == 2) { // base log
                auto base = convert_expression_to_function(Expression(args[1]), x, y, z);
                return [arg, base](double x_value, double y_value, double z_value) {
                    return std::log(arg(x_value, y_value, z_value)) / std::log(base(x_value, y_value, z_value));
                };
            } else { // ln
                return [arg](double x_value, double y_value, double z_value) {
                    return std::log(arg(x_value, y_value, z_value));
                };
            }
        }

        throw std::runtime_error("Unsupported expression type");
    }
    /*
    std::set<double> find_singularities(const Expression& expr, const RCP<const Symbol> &x) {
        std::set<double> singularities;

        if (is_a<Mul>(expr)) {
            auto mul_expr = rcp_static_cast<const Mul>(expr.get_basic());
            for (auto arg : mul_expr->get_args()) {
                if (is_a<Pow>(*arg)) {
                    auto pow_expr = rcp_static_cast<const Pow>(arg);
                    if (pow_expr->get_exp()->__eq__(*SymEngine::minus_one)) {
                        auto denominator = pow_expr->get_base();
                        auto solutions = solve(denominator, x);
                        if (is_a<FiniteSet>(*solutions)) {
                            auto finite_set_casted = rcp_static_cast<const FiniteSet>(solutions);
                            auto elements = finite_set_casted->get_container();
                            for (auto sol : elements) {

                                if (is_a<const Integer>(*sol))
                                    singularities.insert(SymEngine::rcp_static_cast<const SymEngine::Integer>(sol)->as_int());
                                else
                                    singularities.insert(SymEngine::rcp_static_cast<const SymEngine::RealDouble>(sol)->as_double());
                            }
                        }
                    }
                } else {
                    auto sub_singularities = find_singularities(Expression(arg), x);
                    singularities.insert(sub_singularities.begin(), sub_singularities.end());
                }
            }
        }
        
        if (is_a<Log>(expr)) {
            auto log_expr = rcp_static_cast<const Log>(expr.get_basic());
            auto log_argument = log_expr->get_args()[0];
            auto solutions = solve(log_argument, x);
            if (is_a<FiniteSet>(*solutions)) {
                auto finite_set_casted = rcp_static_cast<const FiniteSet>(solutions);
                auto elements = finite_set_casted->get_container();
                for (auto sol : elements) {
                    if (is_a<const Integer>(*sol))
                        singularities.insert(SymEngine::rcp_static_cast<const SymEngine::Integer>(sol)->as_int());
                    else
                        singularities.insert(SymEngine::rcp_static_cast<const SymEngine::RealDouble>(sol)->as_double());
                }
            }
        }

       
        if (is_a<Pow>(expr)) {
            auto pow_expr = rcp_static_cast<const Pow>(expr.get_basic());
            auto exponent = pow_expr->get_exp();
            //std::cout<<*exponent<<std::endl;

            if (is_a<const RealDouble>(*exponent) or is_a<const Integer>(*exponent)) {
                double exp_val;
                if (is_a<const Integer>(*exponent))
                    exp_val=SymEngine::rcp_static_cast<const SymEngine::Integer>(exponent)->as_int();
                else
                    exp_val=SymEngine::rcp_static_cast<const SymEngine::RealDouble>(exponent)->as_double();
        
                if (exp_val < 0 || (exp_val < 2 && std::floor(exp_val) != exp_val)) {
                    auto base = pow_expr->get_base();
                    auto solutions = solve(base, x);
                    if (is_a<FiniteSet>(*solutions)) {
                        auto finite_set_casted = rcp_static_cast<const FiniteSet>(solutions);
                        auto elements = finite_set_casted->get_container();
                        for (auto sol : elements) {
                            if (is_a<const Integer>(*sol))
                                singularities.insert(SymEngine::rcp_static_cast<const SymEngine::Integer>(sol)->as_int());
                            else
                                singularities.insert(SymEngine::rcp_static_cast<const SymEngine::RealDouble>(sol)->as_double());
                        }
                    }
                }
            }
        }

        if (is_a<SymEngine::Add>(expr)) {
            auto add_expr = rcp_static_cast<const SymEngine::Add>(expr.get_basic());
            for (auto arg : add_expr->get_args()) {
                auto sub_singularities = find_singularities(Expression(arg), x);
                singularities.insert(sub_singularities.begin(), sub_singularities.end());
            }
        }

        if (is_a<SymEngine::Mul>(expr) && !is_a<Pow>(expr)) {
            auto mul_expr = rcp_static_cast<const Mul>(expr.get_basic());
            for (auto arg : mul_expr->get_args()) {
                auto sub_singularities = find_singularities(Expression(arg), x);
                singularities.insert(sub_singularities.begin(), sub_singularities.end());
            }
        }

        return singularities;
    }*/

}
/*
template<class T>
inline double evaluate(const Expression & func, T val) {
            
    return (double)func.subs({{x,real_double(val)}}); 

} 
*/
#endif

