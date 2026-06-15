#include <iostream>
#include <ctime>
#include <random>
#include <algorithm>
#include <vector>

using namespace std;

// ===================== STRUCT =====================

struct Saver {
    int age;
    int retirementAge;
    int annualContribution;
    int yearsToRetirement;
    double expectedReturn;   // mu    - from the risk profile
    double volatility;       // sigma - from the risk profile
};

// ===================== PROTOTYPES =====================

void   readInputs(Saver &s);
void   chooseRiskProfile(Saver &s);
int    chooseSimulationCount();
double simulateOnce(Saver &s);
void   drawHistogram(int b1, int b2, int b3, int b4, int b5, int nSims);

// ===================== MAIN =====================

int main()
{
    Saver s;

    cout << "\n\n============================================================";
    cout << "\n        ORIZZONTE - MONTE CARLO SIMULATOR (PROTOTYPE)";
    cout << "\n============================================================\n";
    cout << "\nGoal: simulate retirement-savings accumulation scenarios.\n";
    cout << "\nPress ENTER to start...";
    cin.get();

    // 1) numeric inputs
    readInputs(s);

    // 2) risk profile: the user picks the RISK, we REVEAL the expected return
    chooseRiskProfile(s);

    // 3) number of simulations
    int nSims = chooseSimulationCount();

    // 4) years left until retirement
    s.yearsToRetirement = s.retirementAge - s.age;
    if (s.yearsToRetirement <= 0) {
        cout << "\nRetirement age must be greater than current age. Exiting.\n";
        return 1;
    }

    cout << "\nRunning " << nSims << " simulations over " << s.yearsToRetirement << " years...\n";

    // 5) results vector: EXACTLY nSims cells, sized now (at runtime)
    vector<double> results(nSims);

    for (int i = 0; i < nSims; i++)
        results[i] = simulateOnce(s);

    // 6) sort and read the percentiles
    sort(results.begin(), results.end());

    double unlucky = results[nSims / 10];       // below this lies the worst 10%
    double median  = results[nSims / 2];        // the value in the middle of the line
    double lucky   = results[nSims * 9 / 10];   // above this lies the best 10%

    cout << "\n\n================ YOUR SCENARIOS ================\n";
    cout << "\nUnlucky scenario (worst 10%):   " << (int)unlucky << " EUR";
    cout << "\nTypical scenario (median case): " << (int)median  << " EUR";
    cout << "\nLucky scenario (best 10%):      " << (int)lucky   << " EUR";

    // 7) distribution by buckets
    int b1 = 0, b2 = 0, b3 = 0, b4 = 0, b5 = 0;
    for (int i = 0; i < nSims; i++)
    {
        if (results[i] < 30000)        b1++;
        else if (results[i] < 60000)   b2++;
        else if (results[i] < 90000)   b3++;
        else if (results[i] < 120000)  b4++;
        else                           b5++;
    }

    cout << "\n\n================ PROBABILISTIC RESULTS ================\n";
    cout << "\n0 - 30k:    " << (b1 * 100) / nSims << "%";
    cout << "\n30k - 60k:  " << (b2 * 100) / nSims << "%";
    cout << "\n60k - 90k:  " << (b3 * 100) / nSims << "%";
    cout << "\n90k - 120k: " << (b4 * 100) / nSims << "%";
    cout << "\n120k+:      " << (b5 * 100) / nSims << "%";

    drawHistogram(b1, b2, b3, b4, b5, nSims);

    return 0;
}

// ===================== NUMERIC INPUTS =====================

void readInputs(Saver &s)
{
    cout << "\n\n---------------- YOUR DATA ----------------\n";

    cout << "Current age: ";
    cin >> s.age;

    cout << "Age you want to retire at: ";
    cin >> s.retirementAge;

    cout << "Annual contribution (EUR): ";
    cin >> s.annualContribution;
}

// ===================== RISK PROFILE =====================
// The user chooses HOW MUCH risk to take.
// The expected return is NOT chosen: it is the CONSEQUENCE, and we show it.

void chooseRiskProfile(Saver &s)
{
    int choice;

    cout << "\n\n---------------- RISK PROFILE ----------------\n";
    cout << "Choose how much risk you are willing to take.\n";
    cout << "More risk = higher expected return, but bigger swings.\n\n";
    cout << "1) Conservative (bonds)\n";
    cout << "2) Balanced\n";
    cout << "3) Aggressive   (equities)\n";
    cout << "Choice: ";
    cin >> choice;

    switch (choice)
    {
        case 1: s.expectedReturn = 0.03; s.volatility = 0.05; break;
        case 2: s.expectedReturn = 0.05; s.volatility = 0.10; break;
        case 3: s.expectedReturn = 0.07; s.volatility = 0.15; break;
        default:
            cout << "Invalid choice, using the Balanced profile.\n";
            s.expectedReturn = 0.05; s.volatility = 0.10;
    }

    // THE REVEAL: what this risk level gets you
    cout << "\n-> With this profile you can expect an average return of "
         << (int)(s.expectedReturn * 100) << "% per year,";
    cout << "\n   but be ready for swings of roughly +/- "
         << (int)(s.volatility * 100) << "%.\n";
}

// ===================== NUMBER OF SIMULATIONS =====================

int chooseSimulationCount()
{
    int choice;

    cout << "\n\n---------------- NUMBER OF SIMULATIONS ----------------\n";
    cout << "More simulations = more stable estimate, but slower.\n\n";
    cout << "1) 1,000   (fast, shakier estimate)\n";
    cout << "2) 2,000\n";
    cout << "3) 5,000\n";
    cout << "4) 10,000  (recommended)\n";
    cout << "5) 20,000  (slow, more stable estimate)\n";
    cout << "Choice: ";
    cin >> choice;

    switch (choice)
    {
        case 1: return 1000;
        case 2: return 2000;
        case 3: return 5000;
        case 4: return 10000;
        case 5: return 20000;
        default:
            cout << "Invalid choice, using 10,000 by default.\n";
            return 10000;
    }
}

// ===================== CORE: ONE MONTE CARLO TRIAL =====================

double simulateOnce(Saver &s)
{
    // ONE random engine, shared across all simulations (static = created only once).
    static mt19937 gen(time(NULL));

    // The bell curve uses the expected return and volatility of the chosen profile.
    normal_distribution<double> bell(s.expectedReturn, s.volatility);

    double total = 0.0;

    for (int i = 0; i < s.yearsToRetirement; i++)
    {
        double r = bell(gen);                              // this year's random return, from the bell curve
        total = total * (1.0 + r) + s.annualContribution; // ALL capital grows, then I add the contribution
    }

    return total;
}

// ===================== HISTOGRAM =====================

void drawHistogram(int b1, int b2, int b3, int b4, int b5, int nSims)
{
    cout << "\n\n================ RESULT DISTRIBUTION ================\n";

    // each block represents ~1% of cases (scale adapts to nSims)
    int scale = nSims / 100;
    if (scale < 1) scale = 1;

    cout << "\n0 - 30k      ";
    for (int i = 0; i < b1 / scale; i++) cout << "\u2588";

    cout << "\n30k - 60k    ";
    for (int i = 0; i < b2 / scale; i++) cout << "\u2588";

    cout << "\n60k - 90k    ";
    for (int i = 0; i < b3 / scale; i++) cout << "\u2588";

    cout << "\n90k - 120k   ";
    for (int i = 0; i < b4 / scale; i++) cout << "\u2588";

    cout << "\n120k+        ";
    for (int i = 0; i < b5 / scale; i++) cout << "\u2588";

    cout << "\n==========================================================\n";
}
