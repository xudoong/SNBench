#if !defined(HW_INTEL) && !defined(HW_ARM)
#define HW_INTEL
#endif 

#include "include.h"

#ifdef HW_INTEL
    #ifdef MODE_LATENCY
        #include "intel_fma_latency.h"
    #else 
        #include "intel_fma_throughput.h"
    #endif
    #include "intel_simd_runner.h"
#else 
    #ifdef MODE_LATENCY
        #include "arm_fma_latency.h"
    #else 
        #include "arm_fma_throughput.h"
    #endif
    #include "arm_simd_runner.h"
#endif 


int main(int argc, char **argv)
{
    std::string input_path = "input/input.csv";

#ifdef MODE_LATENCY
    std::string output_path = "result/output_latency.csv";
#else
    std::string output_path = "result/output_throughput.csv";
#endif

    if (argc >= 2) {
        input_path = argv[1];
    }
    if (argc >= 3) {
        output_path = argv[2];
    }

    CSVReader reader(input_path);
    CSVWriter writer(output_path);
    auto lines = reader.readData();

    // warmup
    double warmup_v;
    for (int i = 0; i < 1000000; i++)
        warmup_v += i;

    for (auto line : lines) {
        LineContent lc = reader.parse_line(line);
        if (lc.valid) {
            std::vector<double> cpi_list;

            if (simd_runner::run_all_simd(lc.a, lc.b, lc.c, cpi_list) != -1) {
                std::string reported_fmt = op::get_input_normal_subnormal_format(lc.a, lc.b, lc.c);
                double output = op::comp_op(lc.a, lc.b, lc.c);

                if (lc.format != reported_fmt) {
                    std::cerr << "Error: fmt doesn't match. " << lc.format << " " << reported_fmt << std::endl;
                    exit(-1);
                }
                std::cout << std::scientific << lc.op_name << " " << lc.format << " " << lc.a << " " << lc.b << " " << lc.c << " " << op::comp_op(lc.a, lc.b, lc.c);
                for (auto cpi: cpi_list)
                    std::cout << std::fixed << std::setprecision(1) << " " << cpi;
                std::cout << std::endl;
                writer.writeRow(lc.op_name, lc.format, lc.a, lc.b, lc.c, cpi_list);
            }
        }
    }
    return warmup_v < 0;
}

