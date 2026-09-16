import math
import time
from scipy.stats import norm


def bs_price(S, K, T, r, vol, option_type='call'):
    d1 = (
        math.log(S / K)
        + (r + 0.5 * vol**2) * T
    ) / (vol * math.sqrt(T))

    d2 = d1 - vol * math.sqrt(T)

    if option_type == 'call':
        price = (
            S * norm.cdf(d1)
            - K * math.exp(-r * T) * norm.cdf(d2)
        )
    else:
        price = (
            K * math.exp(-r * T) * norm.cdf(-d2)
            - S * norm.cdf(-d1)
        )

    return price


# Number of options to price
N = 100000

# Same inputs as the C++ tests
S = 100.0
K = 105.0
T = 0.5
r = 0.045
vol = 0.25


# Warm-up
for _ in range(1000):
    bs_price(S, K, T, r, vol, 'call')


# Start benchmark
start = time.perf_counter()

prices = []

for _ in range(N):
    prices.append(
        bs_price(S, K, T, r, vol, 'call')
    )

end = time.perf_counter()


# Results
elapsed = end - start

print(f"Number of prices: {N}")
print(f"Python time: {elapsed:.6f} seconds")
print(f"Average time: {elapsed / N * 1e6:.3f} microseconds")
print(f"First price: {prices[0]:.6f}")
print(f"Last price: {prices[-1]:.6f}")