#include <iostream>   // like `print()` — gives you std::cout
#include <cmath>      // like `math` and `scipy.stats.norm` — gives you erf, exp, log, sqrt
#include <string>     // C++'s text type, std::string (vs Python's built-in str)
#include <stdexcept>  // gives you std::invalid_argument, similar to raising ValueError
#include <map>        // similar to a Python dict — used for bs_greeks' return value

// --- Normal CDF and PDF -----------------------------------------------
// scipy.stats.norm.cdf(x)  ->  norm_cdf(x)
// scipy.stats.norm.pdf(x)  ->  norm_pdf(x)
// There's no built-in normal distribution in <cmath>, but the CDF has a
// closed-form relationship to erf(), the "error function":
double norm_cdf(double x) {
    return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0)));
}

double norm_pdf(double x) {
    return (1.0 / std::sqrt(2.0 * M_PI)) * std::exp(-0.5 * x * x);
}

// --- Pricer --------------------------------------------------------------
// Your Python bs_price() returns a tuple (price, d1, d2). C++ functions can
// only return ONE value by default, so we define a small "struct" — think
// of it as a lightweight, fixed-shape dict/namedtuple — to carry all three
// back together.
struct PriceResult {
    double price;
    double d1;
    double d2;
};

PriceResult bs_price(double S, double K, double T, double r, double vol,
                      const std::string& option_type = "call") {
    double d1 = (std::log(S / K) + (r + 0.5 * vol * vol) * T) / (vol * std::sqrt(T));
    double d2 = d1 - vol * std::sqrt(T);

    double price;
    if (option_type == "call") {
        price = S * norm_cdf(d1) - K * std::exp(-r * T) * norm_cdf(d2);
    } else if (option_type == "put") {
        price = K * std::exp(-r * T) * norm_cdf(-d2) - S * norm_cdf(-d1);
    } else {
        throw std::invalid_argument("option_type must be 'call' or 'put'");
    }

    return PriceResult{price, d1, d2};
}

// --- Greeks ----------------------------------------------------------------
// Same idea: Python returned a dict of 5 values, so we use a struct again.
struct Greeks {
    double delta, gamma, vega, theta, rho;
};

Greeks bs_greeks(double S, double K, double T, double r, double vol,
                  const std::string& option_type = "call") {
    double d1 = (std::log(S / K) + (r + 0.5 * vol * vol) * T) / (vol * std::sqrt(T));
    double d2 = d1 - vol * std::sqrt(T);

    double delta = (option_type == "call") ? norm_cdf(d1) : norm_cdf(d1) - 1.0;
    double gamma = norm_pdf(d1) / (S * vol * std::sqrt(T));
    double vega  = S * norm_pdf(d1) * std::sqrt(T) / 100.0;

    double theta;
    if (option_type == "call") {
        theta = (-S * norm_pdf(d1) * vol / (2 * std::sqrt(T))
                 - r * K * std::exp(-r * T) * norm_cdf(d2)) / 365.0;
    } else {
        theta = (-S * norm_pdf(d1) * vol / (2 * std::sqrt(T))
                 + r * K * std::exp(-r * T) * norm_cdf(-d2)) / 365.0;
    }

    double rho;
    if (option_type == "call") {
        rho = K * T * std::exp(-r * T) * norm_cdf(d2) / 100.0;
    } else {
        rho = -K * T * std::exp(-r * T) * norm_cdf(-d2) / 100.0;
    }

    return Greeks{delta, gamma, vega, theta, rho};
}

// --- Implied volatility (Newton-Raphson) ------------------------------
double implied_vol(double market_price, double S, double K, double T, double r,
                    const std::string& option_type = "call",
                    double initial_guess = 0.2, double tolerance = 1e-6,
                    int max_iterations = 100) {
    double vol = initial_guess;
    for (int i = 0; i < max_iterations; ++i) {
        if (vol <= 0.001 || vol > 5.0) {
            throw std::runtime_error("Vol diverged out of realistic bounds");
        }
        PriceResult r_result = bs_price(S, K, T, r, vol, option_type);
        double vega = S * norm_pdf(r_result.d1) * std::sqrt(T);
        if (vega < 1e-8) {
            throw std::runtime_error("Vega too small, cannot solve");
        }
        double price_diff = market_price - r_result.price;
        if (std::abs(price_diff) < tolerance) {
            return vol;
        }
        vol = vol + price_diff / vega;
    }
    throw std::runtime_error("Implied vol did not converge");
}

// --- Put-call parity check --------------------------------------------
void check_put_call_parity(double S, double K, double T, double r, double vol) {
    double call_price = bs_price(S, K, T, r, vol, "call").price;
    double put_price  = bs_price(S, K, T, r, vol, "put").price;
    double lhs = call_price - put_price;
    double rhs = S - K * std::exp(-r * T);

    std::cout << "C - P = " << lhs << "\n";
    std::cout << "S - K*e^(-rT) = " << rhs << "\n";
    std::cout << "Difference: " << std::abs(lhs - rhs) << "\n";

    if (std::abs(lhs - rhs) >= 1e-6) {
        throw std::runtime_error("Put-call parity violated - check your formulas");
    }
    std::cout << "Put-call parity holds\n";
}

// --- main() : every C++ program needs exactly one of these -------------
// This is your notebook's bottom "example usage" cell. main() is the
// entry point the OS calls when you run the compiled program - Python
// scripts don't need an equivalent because the whole file just runs
// top-to-bottom.
int main() {
    double S = 100, K = 105, T = 0.5, r = 0.045, vol = 0.25;

    PriceResult result = bs_price(S, K, T, r, vol, "call");
    Greeks greeks = bs_greeks(S, K, T, r, vol, "call");

    std::cout << "Call price: $" << result.price << "\n";
    std::cout << "Delta: " << greeks.delta << " Gamma: " << greeks.gamma
               << " Vega: " << greeks.vega << " Theta: " << greeks.theta
               << " Rho: " << greeks.rho << "\n";

    check_put_call_parity(S, K, T, r, vol);

    double iv = implied_vol(result.price, S, K, T, r, "call");
    std::cout << "Recovered implied vol: " << iv << " (should match input vol=" << vol << ")\n";

    return 0;  // 0 tells the OS "program finished with no errors"
}
