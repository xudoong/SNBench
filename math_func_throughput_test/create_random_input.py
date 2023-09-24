import random
import numpy as np
import struct
import math


np.random.seed(0)

MAX_ALLOWED_NORMAL = 1e280 # ignore the larger ones
MAX_ALLOWED_NORMAL_EXPONENT = 1960
MIN_NORMAL = 2.2250738585072014e-308
MAX_SUBNORMAL = 2.224e-308
MIN_SUBNORMAL = 5e-324

N_CASES_PER_SN_CONFIG = 1000


def extract_double(x):
    binary_representation = struct.pack('d', x)
    integer_representation = struct.unpack('Q', binary_representation)[0]
    sign_bit = (integer_representation >> 63) & 1
    mantissa_bits = integer_representation & 0xFFFFFFFFFFFFF
    exponent_bits = (integer_representation >> 52) & 0x7FF
    return sign_bit, mantissa_bits, exponent_bits


def compose_double(sign_bit, mantissa_bits, exponent_bits):
    bits = (sign_bit << 63) | (exponent_bits << 52) | mantissa_bits
    double_value = struct.unpack('d', struct.pack('Q', bits))[0]
    return double_value


def is_subnormal(v):
    v = np.abs(v)
    return v > 0 and v < MAX_SUBNORMAL

def is_normal(x):
    x = np.abs(x)
    return x > MIN_NORMAL and x < MAX_ALLOWED_NORMAL


def fp_classify(x):
    if is_normal(x):
        return 'N'
    if is_subnormal(x):
        return 'S'
    if x == 0:
        return 'O'
    return 'X'


def get_sn_config(output, a, b=None):
    if b is None:
        return fp_classify(output) + fp_classify(a)
    return fp_classify(output) + fp_classify(a) + fp_classify(b)


def random_double_normal(min_exp=1, max_exp=MAX_ALLOWED_NORMAL_EXPONENT, neg=False):
    sig = 0
    min_exp = max(1, min_exp)
    if neg == True and np.random.randint(100) % 2 == 0:
        sig = 1
    man = np.random.randint(0, 2 ** 52, dtype=np.int64) & 0xFFFFFFFFFFFFF
    exp = np.random.randint(min_exp, max_exp) & 0x7FF
    return compose_double(sig, man, exp)


def random_double_subnormal(neg=False):
    sig = 1
    if neg == True and np.random.randint(100) % 2 == 0:
        sig = -1
    man = np.random.randint(0, 2 ** 52, dtype=np.int64) & 0xFFFFFFFFFFFFF
    exp = 0
    return sig * compose_double(0, man, exp)


def is_vaild(sn_config, output, a, b=None):
     return get_sn_config(output, a, b) == sn_config.upper()


class BaseOp:
    
    def __init__(self) -> None:
        self.name = 'None'

    def check_vaild_and_log(self, sn_config, out, a, b=None):
        rv = is_vaild(sn_config, out, a, b)
        if rv:
            txt = f'{self.name},{sn_config.upper()},{out},{a}' + ('' if b is None else f',{b}')
            print(txt)
        return rv
    

#===========================================================
#   POW
#===========================================================
def safe_pow(a, b):
    try:
        ans = math.pow(a, b)
    except OverflowError:
        ans = float('inf')
    except ValueError:
        ans = float('nan')
    return ans
    

class PowOp(BaseOp):

    def __init__(self) -> None:
        self.name = 'POW'

    def pow_nnn(self):
        sn_config = 'nnn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = np.random.uniform(-30, 30)
            b = np.random.uniform(-30, 30)
            out = safe_pow(a, b)
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def pow_nsn(self):
        sn_config = 'nsn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_subnormal()
            b = np.random.uniform(-30, 30)
            out = safe_pow(a, b)
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def pow_nns(self):
        sn_config = 'nns'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = np.random.uniform(-30, 30)
            b = random_double_subnormal()
            out = safe_pow(a, b)
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def pow_nss(self):
        sn_config = 'nss'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_subnormal()
            b = random_double_subnormal()
            out = safe_pow(a, b)
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def pow_snn(self):
        sn_config = 'snn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_normal()
            abs_exp_a = abs(extract_double(a)[2] - 1023)
            if abs_exp_a == 0:
                continue
            exp_ub = math.log2(1022 / abs_exp_a) + 1023
            exp_lb = math.log2((1022 + 52) / abs_exp_a) + 1023
            exp_ub, exp_lb = max(exp_ub, exp_lb) + 1, min(exp_ub, exp_lb)
            b = random_double_normal(min_exp=exp_lb, max_exp=exp_ub)
            out = safe_pow(a, b)
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def pow_sns(self):
        # POW-SNS is an impossible sn-config"
        pass

    def pow_ssn(self):
        sn_config = 'ssn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_subnormal()
            b = np.random.uniform(-2, 2)
            out = safe_pow(a, b)
            count += self.check_vaild_and_log(sn_config, out, a, b)

    def pow_sss(self):
        # POW-SSS is an impossible sn-config"
        pass

    def pow_run_all(self):
        self.pow_nnn()
        self.pow_nsn()
        self.pow_nns()
        self.pow_nss()
        self.pow_snn()
        self.pow_sns()
        self.pow_ssn()
        self.pow_sss()


#===========================================================
#   EXP
#===========================================================
def safe_exp(a):
    try:
        ans = math.exp(a)
    except OverflowError:
        ans = float('inf')
    return ans


class ExpOp(BaseOp):

    def __init__(self) -> None:
        self.name = 'EXP'

    def exp_nn(self):
        sn_config = 'nn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = np.random.uniform(-700, 700)  
            out = safe_exp(a)
            count += self.check_vaild_and_log(sn_config, out, a)

    def exp_ns(self):
        sn_config = 'ns'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_subnormal()
            out = safe_exp(a)
            count += self.check_vaild_and_log(sn_config, out, a)

    def exp_sn(self):
        sn_config = 'sn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = np.random.uniform(-750, -708)
            out = safe_exp(a)
            count += self.check_vaild_and_log(sn_config, out, a)

    def exp_ss(self):
        # EXP-SS is an impossible sn-config
        pass

    def exp_run_all(self):
        self.exp_nn()
        self.exp_ns()
        self.exp_sn()


#===========================================================
#   LOG10
#===========================================================
class LogOp(BaseOp):

    def __init__(self) -> None:
        self.name = 'LOG'

    def log_nn(self):
        sn_config = 'nn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_normal()
            out = math.log(a)
            count += self.check_vaild_and_log(sn_config, out, a)

    def log_ns(self):
        sn_config = 'ns'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_subnormal()
            out = math.log(a)
            count += self.check_vaild_and_log(sn_config, out, a)

    def log_sn(self):
        # log-SN is an impossible sn-config"
        pass

    def log_ss(self):
        # log-SS is an impossible sn-config"
        pass

    def log_run_all(self):
        self.log_nn()
        self.log_ns()


#===========================================================
#   SIN
#===========================================================
class SinOp(BaseOp):

    def __init__(self) -> None:
        self.name = 'SIN'

    def sin_nn(self):
        sn_config = 'nn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = np.random.uniform(0, 6.28)
            out = math.sin(a)
            count += self.check_vaild_and_log(sn_config, out, a)


    def sin_ss(self):
        sn_config = 'ss'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_subnormal()
            out = math.sin(a)
            assert(out == a)
            count += self.check_vaild_and_log(sn_config, out, a)


    def sin_run_all(self):
        self.sin_nn()
        self.sin_ss()


#===========================================================
#   COS
#===========================================================
class CosOp(BaseOp):

    def __init__(self) -> None:
        self.name = 'COS'

    def cos_nn(self):
        sn_config = 'nn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = np.random.uniform(0, 6.28)
            out = math.cos(a)
            count += self.check_vaild_and_log(sn_config, out, a)


    def cos_ns(self):
        sn_config = 'ns'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_subnormal()
            out = math.cos(a)
            assert(out == 1)
            count += self.check_vaild_and_log(sn_config, out, a)


    def cos_run_all(self):
        self.cos_nn()
        self.cos_ns()


#===========================================================
#   ATAN
#===========================================================
class TanOp(BaseOp):

    def __init__(self) -> None:
        self.name = 'TAN'

    def tan_nn(self):
        sn_config = 'nn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = np.random.uniform(0, 6.28)
            out = math.tan(a)
            count += self.check_vaild_and_log(sn_config, out, a)

    def tan_ss(self):
        sn_config = 'ss'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a = random_double_subnormal()
            out = math.tan(a)
            count += self.check_vaild_and_log(sn_config, out, a)

    def tan_run_all(self):
        self.tan_nn()
        self.tan_ss()


def main():
    PowOp().pow_run_all()
    ExpOp().exp_run_all()
    LogOp().log_run_all()
    SinOp().sin_run_all()
    CosOp().cos_run_all()
    TanOp().tan_run_all()


if __name__ == '__main__':
    main()