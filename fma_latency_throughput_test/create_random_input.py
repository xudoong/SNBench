import random
import numpy as np
import struct
import math


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


class MulSampler:

    @staticmethod
    def sample_one_input(s_or_n, normal_min_exp=1, normal_max_exp=MAX_ALLOWED_NORMAL_EXPONENT):
        if s_or_n.upper() == 'S':
            return random_double_subnormal()
        return random_double_normal(min_exp=normal_min_exp, max_exp=normal_max_exp)
    
    @staticmethod
    def narrow_sn_config(sn_config):
        if len(sn_config) == 5:
            sn_config = sn_config[1:-1]
        return sn_config.upper()
    
    @staticmethod
    def sample_default(sn_config):
        sn_config = MulSampler.narrow_sn_config(sn_config)
        while True:
            input1 = MulSampler.sample_one_input(sn_config[1])
            input2 = MulSampler.sample_one_input(sn_config[2])
            output = input1 * input2
            if fp_classify(output) == sn_config[0].upper():
                return input1, input2
            
    @staticmethod
    def sample_with_constrained_normal_output(sn_config, output_min_exp, output_max_exp):
        sn_config = MulSampler.narrow_sn_config(sn_config)
        assert sn_config[0] == 'N'
        assert sn_config[1:] != 'SS'

        while True:
            if sn_config[1:] == 'NS' or sn_config[1:] == 'SN': # one of the input is a subnormal number
                input1 = MulSampler.sample_one_input('S')
            else:
                exp1_max = min(output_max_exp + 1023 - 1, MAX_ALLOWED_NORMAL_EXPONENT)
                input1 = MulSampler.sample_one_input('N', normal_max_exp=exp1_max)
            '''
            exp_out - 1023 = exp_input1 - 1023 + exp_input2 - 1023 =>
            exp_input2 = exp_out - exp_input1 + 1023
            '''
            exp1 = extract_double(input1)[2]
            input2_min_exp = output_min_exp - exp1 + 1023
            input2_max_exp = output_max_exp - exp1 + 1023
            input2_normal = MulSampler.sample_one_input('N', normal_min_exp=input2_min_exp, normal_max_exp=input2_max_exp)
            output = input1 * input2_normal
            exp_out = extract_double(output)[2]
            if exp_out >= output_min_exp and exp_out < output_max_exp: # if the output is vaild
                if sn_config[1:] == 'NS':
                    return input2_normal, input1 
                else:
                    return input1, input2_normal 

    @staticmethod
    def sample_snn(sn_config=None, min_exp=-53, max_exp=0):
        while True:
            a = random_double_normal(max_exp=1023)
            # clip the range to speed up sampling. the vaild range is unchanged
            exp_a = extract_double(a)[2]
            min_exp_b = min_exp - exp_a + 1023
            max_exp_b = max_exp - exp_a + 1023 + 2
            b = random_double_normal(min_exp=min_exp_b, max_exp=max_exp_b)
            if is_subnormal(a * b):
                return a, b
            
    @staticmethod
    def sample_ssn(sn_config=None, min_exp=-53, max_exp=0):
        while True:
            a = random_double_subnormal()
            # clip the range to speed up sampling. the vaild range is unchanged
            exp_a = extract_double(a)[2]
            min_exp_b = min_exp - exp_a + 1023
            max_exp_b = max_exp - exp_a + 1023 + 2
            b = random_double_normal(min_exp=min_exp_b, max_exp=max_exp_b)
            if is_subnormal(a * b):
                return a, b

    @staticmethod
    def sample_oss(sn_config=None):
        a = random_double_subnormal()
        b = random_double_subnormal()
        return a, b  


class FmaOp:

    def __init__(self) -> None:
        self.name = 'FMA'

    @staticmethod
    def get_sn_config(a, b, c):
        output = a * b + c 
        mul_output = a * b
        return fp_classify(output) + fp_classify(mul_output) + fp_classify(a) + fp_classify(b) + fp_classify(c)

    @staticmethod
    def is_fma_vaild(sn_config, a, b, c):
     return FmaOp.get_sn_config(a, b, c) == sn_config.upper()

    def check_vaild_and_log(self, sn_config, a, b, c):
        rv = FmaOp.is_fma_vaild(sn_config, a, b, c)
        if rv:
            txt = f'{self.name},{sn_config.upper()},{a * b + c},{a * b},{a},{b},{c}'
            print(txt)
        return rv
    
    # ===========================================================
    # First 8 cases: nxxxn
    # ======================
    def fma_nnnnn(self): #1 1
        sn_config = 'nnnnn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a, b = MulSampler.sample_default(sn_config)
            c = random_double_normal()
            count += self.check_vaild_and_log(sn_config, a, b, c)

    def fma_nnsnn(self): #2 2
        sn_config = 'nnsnn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a, b = MulSampler.sample_default(sn_config)
            c = random_double_normal()
            count += self.check_vaild_and_log(sn_config, a, b, c)

    def fma_nnnsn(self): #3 3
        '''same as above, ignore this'''
        pass

    def fma_nnssn(self): #4 3
        '''impossible'''
        pass

    def fma_nsnnn(self): #5 3
        sn_config = 'nsnnn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a, b = MulSampler.sample_snn(sn_config)
            c = random_double_normal()
            count += self.check_vaild_and_log(sn_config, a, b, c)

    def fma_nssnn(self): #6 4
        sn_config = 'nssnn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a, b = MulSampler.sample_ssn()
            c = random_double_normal()
            count += self.check_vaild_and_log(sn_config, a, b, c)

    def fma_nsnsn(self): #7 4
        '''same as above, ignore this'''
        pass

    def fma_nossn(self): #8 5
        sn_config = 'nossn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a, b = MulSampler.sample_oss()
            c = random_double_normal()
            count += self.check_vaild_and_log(sn_config, a, b, c)

    # ===========================================================
    # Second 8 cases: nxxxs
    # ======================
    def fma_nnnns(self): #9 6
        sn_config = 'nnnns'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a, b = MulSampler.sample_default(sn_config)
            c = random_double_subnormal()
            count += self.check_vaild_and_log(sn_config, a, b, c)

    def fma_nnsns(self): #10 7
        sn_config = 'nnsns'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a, b = MulSampler.sample_default(sn_config)
            c = random_double_subnormal()
            count += self.check_vaild_and_log(sn_config, a, b, c)

    def fma_nnnss(self): #11 7
        '''same as above, ignore this'''
        pass

    def fma_nnsss(self): #12 7
        '''impossible'''
        pass

    def fma_nsnns(self): #13 8
        sn_config = 'nsnns'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a, b = MulSampler.sample_snn(sn_config)
            c = random_double_subnormal()
            count += self.check_vaild_and_log(sn_config, a, b, c)

    def fma_nssns(self): #14 9
        sn_config = 'nssns'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a, b = MulSampler.sample_ssn()
            c = random_double_subnormal()
            count += self.check_vaild_and_log(sn_config, a, b, c)

    def fma_nsnss(self): #15 9
        '''same as above, ignore this'''
        pass

    def fma_nosss(self): #16 9
        '''impossible'''
        pass

    # ===========================================================
    # Third 8 cases: sxxxn (The most compilcated cases)
    # ======================
    def fma_snnnn(self): #17 10
        sn_config = 'snnnn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            '''
            in order to get Subnormal = Normal + Normal, the exp of the two normal must be smaller than 2^(53-1023). To speed up sampling, narrow down the range from 53 to 20
            '''
            BITS = 20
            a, b = MulSampler.sample_with_constrained_normal_output(sn_config, 1, BITS)
            '''make c negative'''
            c = -random_double_normal(max_exp=BITS)
            count += self.check_vaild_and_log(sn_config, a, b, c)

    def fma_snsnn(self): #18 11
        sn_config = 'snsnn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            BITS = 20
            a, b = MulSampler.sample_with_constrained_normal_output(sn_config, 1, BITS)
            c = -random_double_normal(max_exp=BITS)
            count += self.check_vaild_and_log(sn_config, a, b, c)

    def fma_snnsn(self): #19 11
        '''same as above, ignore this'''
        pass

    def fma_snssn(self): #20 11
        '''impossible'''
        pass

    def fma_ssnnn(self): #21 12
        sn_config = 'ssnnn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a, b = MulSampler.sample_snn(sn_config)
            c = -random_double_normal(max_exp=2)
            count += self.check_vaild_and_log(sn_config, a, b, c)

    def fma_sssnn(self): #22 13
        sn_config = 'sssnn'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a, b = MulSampler.sample_ssn()
            c = -random_double_normal(max_exp=2)
            count += self.check_vaild_and_log(sn_config, a, b, c)

    def fma_ssnsn(self): #23 13
        '''same as above, ignore this'''
        pass

    def fma_sossn(self): #24 13
        '''impossible'''
        pass

    # ===========================================================
    # Third 8 cases: sxxxn (The most compilcated cases)
    # ======================
    def fma_snnns(self): #25 14
        sn_config = 'snnns'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            '''
            in order to get Subnormal = Normal + Subormal, the exp of the normal input must be smaller than 2^(2-1023).
            '''
            BITS = 2
            a, b = MulSampler.sample_with_constrained_normal_output(sn_config, 1, BITS)
            '''make c negative'''
            c = -random_double_subnormal()
            count += self.check_vaild_and_log(sn_config, a, b, c)

    def fma_snsns(self): #26 15
        sn_config = 'snsns'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            BITS = 2
            a, b = MulSampler.sample_with_constrained_normal_output(sn_config, 1, BITS)
            c = -random_double_subnormal()
            count += self.check_vaild_and_log(sn_config, a, b, c)

    def fma_snnss(self): #27 15
        '''same as above, ignore this'''
        pass

    def fma_snsss(self): #28 15
        '''impossible'''
        pass

    def fma_ssnns(self): #29 16
        sn_config = 'ssnns'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a, b = MulSampler.sample_snn(sn_config)
            c = random_double_subnormal(neg=True)
            count += self.check_vaild_and_log(sn_config, a, b, c)

    def fma_sssns(self): #30 17
        sn_config = 'sssns'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a, b = MulSampler.sample_ssn()
            c = random_double_subnormal(neg=True)
            count += self.check_vaild_and_log(sn_config, a, b, c)

    def fma_ssnss(self): #23 17
        '''same as above, ignore this'''
        pass

    def fma_sosss(self): #24 18
        sn_config = 'sosss'
        count = 0
        while count < N_CASES_PER_SN_CONFIG:
            a, b = MulSampler.sample_oss()
            c = random_double_subnormal()
            count += self.check_vaild_and_log(sn_config, a, b, c)

    def run_all(self):
        # ----------------
        # nxxxn
        self.fma_nnnnn()
        self.fma_nnsnn()
        self.fma_nnnsn()
        self.fma_nnssn()
        self.fma_nsnnn()
        self.fma_nssnn()
        self.fma_nsnsn()
        self.fma_nossn()
        # ----------------
        # nxxxs
        self.fma_nnnns()
        self.fma_nnsns()
        self.fma_nnnss()
        self.fma_nnsss()
        self.fma_nsnns()
        self.fma_nssns()
        self.fma_nsnss()
        self.fma_nosss()
        # ----------------
        # sxxxn
        self.fma_snnnn()
        self.fma_snsnn()
        self.fma_snnsn()
        self.fma_snssn()
        self.fma_ssnnn()
        self.fma_sssnn()
        self.fma_ssnsn()
        self.fma_sossn()
        # ----------------
        # sxxxs
        self.fma_snnns()
        self.fma_snsns()
        self.fma_snnss()
        self.fma_snsss()
        self.fma_ssnns()
        self.fma_sssns()
        self.fma_ssnss()
        self.fma_sosss()


if __name__ == '__main__':
    FmaOp().run_all()
