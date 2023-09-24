#if !defined(HW_INTEL) && !defined(HW_ARM)
#define HW_INTEL
#endif 

#include "include.h"

#ifdef HW_INTEL
    #if defined(OP_ADD)
    #include "intel/add.h"
    #elif defined(OP_MUL)
    #include "intel/mul.h"
    #elif defined(OP_DIV)
    #include "intel/div.h"
    #elif defined(OP_SQRT)
    #include "intel/sqrt.h"
    #else 
    #include "intel/add.h"
    #endif
    #include "intel/simd_runner.h"
#else 
    #if defined(OP_ADD)
    #include "arm/add.h"
    #elif defined(OP_MUL)
    #include "arm/mul.h"
    #elif defined(OP_DIV)
    #include "arm/div.h"
    #elif defined(OP_SQRT)
    #include "arm/sqrt.h"
    #else 
    #include "arm/add.h"
    #endif
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
    CSVReader reader(input_path);
    CSVWriter writer(output_path);
    auto lines = reader.readData();

    // warmup
    double warmup_v;
    for (int i = 0; i < 1000000; i++)
        warmup_v += i;

    for (auto line : lines) {
        LineContent lc = reader.parse_line(line);
        if (lc.valid && lc.op_name == op::NAME) {
            std::vector<double> cpi_list;
            std::cout << lc.op_name << " " << lc.format << " " << lc.a << " " << lc.b << std::endl;

            if (simd_runner::run_all_simd(lc.a, lc.b, cpi_list) != -1) {
                std::string reported_fmt = op::get_input_normal_subnormal_format(lc.a, lc.b);
                double output = op::comp_op(lc.a, lc.b);

                if (lc.format != reported_fmt) {
                    std::cout << "Warning: fmt doesn't match." << std::endl;
                }
                writer.writeRow(lc.op_name, lc.format, reported_fmt, lc.a, lc.b, output, cpi_list);
            }
        }
    }
    return warmup_v < 0;
}

