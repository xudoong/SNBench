#include "include.h"

#if defined(HW_INTEL)
    #if defined(OP_POW)
    #include "intel/pow.h"
    #elif defined(OP_LOG)
    #include "intel/log.h"
    #elif defined(OP_EXP)
    #include "intel/exp.h"
    #elif defined(OP_SIN)
    #include "intel/sin.h"
    #elif defined(OP_COS)
    #include "intel/cos.h"
    #elif defined(OP_TAN)
    #include "intel/tan.h"
    #endif
#elif defined(HW_ARM)
    #if defined(OP_POW)
    #include "arm/pow.h"
    #elif defined(OP_LOG)
    #include "arm/log.h"
    #elif defined(OP_EXP)
    #include "arm/exp.h"
    #elif defined(OP_SIN)
    #include "arm/sin.h"
    #elif defined(OP_COS)
    #include "arm/cos.h"
    #elif defined(OP_TAN)
    #include "arm/tan.h"
    #endif
#endif

#if defined(HW_INTEL)
#include "intel/simd_runner.h"
#elif defined(HW_ARM)
#include "arm/simd_runner.h"
#endif


int main(int argc, char **argv)
{
    std::string input_path = "input/input.csv";
    std::string output_path = "result/output.csv";
    if (argc >= 2) {
        input_path = argv[1];
    }
    if (argc >= 3) {
        output_path = argv[2];
    }

    std::cout << "======================" << std::endl;
    std::cout << "OP=" << op::NAME << std::endl;
    CSVWriter writer(output_path);

    // init
    std::vector<std::string> op_order;
    std::map<std::string, InputData> input_map;
    RandomInputInit::init_from_csv(input_path, op::NAME, op_order, input_map);

    // warmup
    double warmup_v;
    for (int i = 0; i < 1000000; i++)
        warmup_v += i;

    for (auto op_fmt : op_order) {
        auto input = input_map[op_fmt];
        std::vector<double> cpi_list;
        simd_runner::run_all_simd(input.a_list, input.b_list, input.size, cpi_list);
        std::string reported_fmt = op::get_input_normal_subnormal_format(input.a_list[0], input.b_list[0]);
        if (reported_fmt != op_fmt.substr(op_fmt.length() - reported_fmt.length(), reported_fmt.length())) {
            std::cerr << "warning: fmt doesn't match: " << op_fmt << " " << reported_fmt << std::endl;
        }
        std::cout << op_fmt << " ";
        for (auto cpi : cpi_list)
            std::cout << std::setw(6) << std::fixed << std::setprecision(1) << cpi << " ";
        std::cout << std::endl;
        writer.writeRow(op_fmt, cpi_list);
    }
    return warmup_v < 0;
}

