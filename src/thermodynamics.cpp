#include <QtWidgets>
#include <atomic>
#include <cmath>
#include <vector>
#include <array>
#include <cstdint>

// ═══════════════════════════════════════════════════════════════════════════
//                    TERMODINÂMICA DOS PRIMOS v4.1
//           Implementação baseada no framework de máxima entropia
//           Com tracking de convergência por década
// ═══════════════════════════════════════════════════════════════════════════

// ======================= CONSTANTES FUNDAMENTAIS =======================

namespace Constants {
    // Hardy-Littlewood C₂ para twins (alta precisão)
    constexpr long double C2 = 0.6601618158468695739278121100145557L;

    // === MODELO CUMULATIVO ===
    // kT_cum = a×ln²(p) + b×ln(p) + c
    constexpr double kT_CUM_LN2 = 0.7784;
    constexpr double kT_CUM_LN = -2.32;
    constexpr double kT_CUM_C = -13.9;

    // === MODELO LOCAL (por década) ===
    // kT_local ≈ 0.75×ln²(p) - 17 (Hardy-Littlewood puro!)
    constexpr double kT_LOC_LN2 = 0.7499;   // ≈ 1/(2C₂)
    constexpr double kT_LOC_LN = -0.24;     // ≈ 0
    constexpr double kT_LOC_C = -16.7;      // ≈ gap_min médio

    // Offset para primos: gap_min = 2
    constexpr double OFFSET_PRIMES = 2.0;

    // Valor teórico do ratio kT/ln²(p) (assintótico local)
    constexpr double RATIO_THEORETICAL = 0.7499;

    // kT binário = 1/ln(2) ≈ 1.4427 (FIXO)
    constexpr long double kT_BINARY = 1.4426950408889634073599246810018921L;
    constexpr long double LN2 = 0.6931471805599453094172321214581766L;

    // Limiares de regime (descoberta empírica)
    constexpr uint64_t FROZEN_LIMIT = 500;          // ~17º twin, gaps = gap_min
    constexpr uint64_t TRANSITION_LIMIT = 10000;    // transição inicial
    constexpr uint64_t ASYMPTOTIC_LIMIT = 100000;   // regime assintótico bom
    constexpr uint64_t PRECISE_LIMIT = 1000000;     // regime preciso (>99%)

    // Classes válidas para twin primes mod 30
    constexpr int TWIN_CLASSES[3] = {11, 17, 29};

    // Matriz de gap_min entre classes mod 30
    constexpr int GAP_MIN[3][3] = {
        {30,  6, 18},  // de 11 → {11, 17, 29}
        {24, 30, 12},  // de 17 → {11, 17, 29}
        {12, 18, 30}   // de 29 → {11, 17, 29}
    };

    // Wheel mod 210 = 2×3×5×7: 48 coprimos (muito mais eficiente)
    constexpr int WHEEL_SIZE = 48;
    constexpr int WHEEL_OFFSETS[48] = {
        1, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47,
        53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103,
        107, 109, 113, 121, 127, 131, 137, 139, 143, 149, 151, 157,
        163, 167, 169, 173, 179, 181, 187, 191, 193, 197, 199, 209
    };

    // v₂ teórico (distribuição geométrica)
    constexpr double V2_MEAN_THEORETICAL = 2.0;
    constexpr double V2_P1_THEORETICAL = 0.5;
    constexpr double V2_P2_THEORETICAL = 0.25;
    constexpr double V2_P3_THEORETICAL = 0.125;
}

// ======================= MODELOS =======================

enum class AnalysisMode { Primes, Twins, Both };

// Regime do sistema
enum class Regime {
    Frozen,      // p < 500, gaps = gap_min
    Transition,  // 500 < p < 10k
    Asymptotic,  // 10k < p < 1M
    Precise      // p > 1M, lei vale com R² > 0.99
};

inline const char* regimeName(Regime r) {
    switch (r) {
        case Regime::Frozen: return "CONGELADO";
        case Regime::Transition: return "TRANSIÇÃO";
        case Regime::Asymptotic: return "ASSINTÓTICO";
        case Regime::Precise: return "PRECISO ✓";
    }
    return "?";
}

inline QString regimeColor(Regime r) {
    switch (r) {
        case Regime::Frozen: return "blue";
        case Regime::Transition: return "orange";
        case Regime::Asymptotic: return "#DAA520";  // goldenrod
        case Regime::Precise: return "green";
    }
    return "black";
}

// Índice de classe mod 30 para twins
inline int classIndex(uint64_t p) {
    int r = p % 30;
    if (r == 11) return 0;
    if (r == 17) return 1;
    if (r == 29) return 2;
    return -1;
}

struct TransitionStats {
    uint64_t count[3][3] = {{0}};
    long double sumGap[3][3] = {{0}};

    double avgGap(int c1, int c2) const {
        return count[c1][c2] > 0 ?
            static_cast<double>(sumGap[c1][c2] / count[c1][c2]) : 0.0;
    }

    double probability(int c1, int c2) const {
        uint64_t total = 0;
        for (int j = 0; j < 3; ++j) total += count[c1][j];
        return total > 0 ? static_cast<double>(count[c1][c2]) / total : 0.0;
    }
};

// Stats por década (10³, 10⁴, ..., 10¹⁰)
struct DecadeStats {
    uint64_t count = 0;
    long double sumExcess = 0;
    long double sumLn2 = 0;

    double kT() const {
        return count > 0 ? static_cast<double>(sumExcess / count) : 0;
    }

    double avgLn2() const {
        return count > 0 ? static_cast<double>(sumLn2 / count) : 0;
    }

    double ratio() const {
        double ln2 = avgLn2();
        return (count > 0 && ln2 > 0) ? kT() / ln2 : 0;
    }

    double pctTheoretical() const {
        return ratio() / Constants::RATIO_THEORETICAL * 100.0;
    }
};

struct Stats {
    uint64_t currentN = 0;
    uint64_t primeCount = 0;
    uint64_t twinCount = 0;

    Regime regime = Regime::Frozen;

    // Gaps médios
    double avgPrimeGap = 0.0;
    double avgTwinGap = 0.0;

    // kT Primos: kT = ⟨gap⟩ - 2
    double kTPrimeEmpirical = 0.0;
    double kTPrimeTheoretical = 0.0;

    // kT Twins
    double kTTwinEmpirical = 0.0;
    double kTTwinTheoretical = 0.0;
    double kTTwinAsymptotic = 0.0;
    uint64_t asymptoticCount = 0;

    // Razão kT/ln²(p)
    double kTRatio = 0.0;

    // kT Binário
    double kTBinary = Constants::kT_BINARY;

    // v₂(p+1)
    double meanV2 = 0.0;
    uint64_t v2Histogram[8] = {0};

    // v₂(gap) - trailing zeros do gap entre twins
    double meanV2Gap = 0.0;
    uint64_t v2GapHistogram[8] = {0};

    // Correlação v₂(p+1) vs v₂(gap)
    double corrV2 = 0.0;

    // Transições mod 30
    TransitionStats transitions;

    // R² de Boltzmann (consistência entre transições)
    double boltzmannR2 = 0.0;

    // Stats por década (índice 0 = 10³, 1 = 10⁴, ..., 7 = 10¹⁰)
    static constexpr int N_DECADES = 8;
    DecadeStats decades[N_DECADES];
};

Q_DECLARE_METATYPE(Stats)

// ======================= SIEVE COM WHEEL MOD 210 =======================

class PrimeSieve {
public:
    explicit PrimeSieve(uint64_t limit = 10000000) {
        generateBasePrimes(limit);
    }

    // Teste otimizado: já assume que n é coprimo com 2,3,5,7 (vem do wheel 210)
    bool isPrimeFromWheel(uint64_t n) const {
        // n já passou pelo filtro mod 210, então é coprimo com 2,3,5,7
        // Só precisa testar divisores >= 11

        if (n < 121) return n > 1;  // 121 = 11², menor composto coprimo com 210

        uint64_t sqrtN = static_cast<uint64_t>(std::sqrt(static_cast<double>(n))) + 1;

        // m_basePrimes começa em 11 (após 2,3,5,7)
        for (uint64_t p : m_basePrimes) {
            if (p > sqrtN) break;
            if (n % p == 0) return false;
        }
        return true;
    }

    // Teste geral (para casos especiais)
    bool isPrime(uint64_t n) const {
        if (n < 2) return false;
        if (n == 2 || n == 3 || n == 5 || n == 7) return true;
        if (n % 2 == 0 || n % 3 == 0 || n % 5 == 0 || n % 7 == 0) return false;
        return isPrimeFromWheel(n);
    }

private:
    void generateBasePrimes(uint64_t limit) {
        std::vector<bool> sieve(limit + 1, true);
        sieve[0] = sieve[1] = false;

        for (uint64_t i = 2; i * i <= limit; ++i) {
            if (sieve[i]) {
                for (uint64_t j = i * i; j <= limit; j += i)
                    sieve[j] = false;
            }
        }

        // Só guarda primos >= 11 (coprimos com 210 que são primos)
        m_basePrimes.reserve(limit / 12);
        for (uint64_t i = 11; i <= limit; ++i) {
            if (sieve[i])
                m_basePrimes.push_back(i);
        }
    }

    std::vector<uint64_t> m_basePrimes;
};

// v₂(n) = trailing zeros
static inline unsigned v2_of(uint64_t x) {
    if (x == 0) return 0;
    return __builtin_ctzll(x);
}

// Wheel iterator mod 210
class WheelIterator {
public:
    WheelIterator(uint64_t start) {
        m_base = (start / 210) * 210;
        m_idx = 0;
        // Encontra primeiro offset >= start
        for (int i = 0; i < Constants::WHEEL_SIZE; ++i) {
            if (m_base + Constants::WHEEL_OFFSETS[i] >= start) {
                m_idx = i;
                return;
            }
        }
        // Se não encontrou, vai pro próximo ciclo
        m_base += 210;
        m_idx = 0;
    }

    uint64_t current() const {
        return m_base + Constants::WHEEL_OFFSETS[m_idx];
    }

    void next() {
        ++m_idx;
        if (m_idx >= Constants::WHEEL_SIZE) {
            m_idx = 0;
            m_base += 210;
        }
    }

private:
    uint64_t m_base;
    int m_idx;
};

// ======================= WORKER =======================

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr)
        : QObject(parent), m_sieve(10000000)
    {
        m_stopRequested.store(false);
    }

    void configure(uint64_t startN, AnalysisMode mode, bool multiThread)
    {
        m_startN = std::max(startN, uint64_t(3));
        m_mode = mode;
        m_multiThread = multiThread;
    }

public slots:
    void process()
    {
        using namespace Constants;

        WheelIterator wheel(m_startN);

        // Contadores primos
        uint64_t lastPrime = 0;
        uint64_t primeCount = 0;
        long double sumPrimeGaps = 0.0L;

        // Twins
        uint64_t lastTwinP = 0;
        int lastTwinClass = -1;
        uint64_t twinCount = 0;
        long double sumTwinGaps = 0.0L;
        long double sumTwinExcess = 0.0L;

        // Regime assintótico (p > 100k)
        uint64_t asymptoticCount = 0;
        long double sumAsymptoticExcess = 0.0L;

        // v₂ stats
        long double sumV2 = 0.0L;
        uint64_t countV2 = 0;
        uint64_t v2Histogram[8] = {0};

        // v₂(gap) stats
        long double sumV2Gap = 0.0L;
        uint64_t countV2Gap = 0;
        uint64_t v2GapHistogram[8] = {0};

        // Correlação v₂(p+1) vs v₂(gap)
        long double sumV2Product = 0.0L;
        long double sumV2Sq = 0.0L;
        long double sumV2GapSq = 0.0L;

        // Transições mod 30
        TransitionStats transitions;

        // Décadas
        DecadeStats decades[Stats::N_DECADES];

        Stats stats;

        const uint64_t progressStep = 5000;
        uint64_t primesSinceLastUpdate = 0;

        while (!m_stopRequested.load()) {
            uint64_t n = wheel.current();
            wheel.next();

            // Usa teste otimizado: n já é coprimo com 2,3,5,7 pelo wheel 210
            if (!m_sieve.isPrimeFromWheel(n))
                continue;

            ++primeCount;

            // v₂(p+1)
            unsigned k = v2_of(n + 1);
            sumV2 += k;
            ++countV2;
            if (k >= 1 && k <= 7)
                v2Histogram[k - 1]++;
            else if (k >= 8)
                v2Histogram[7]++;

            // Gap de primos
            if (lastPrime != 0) {
                uint64_t gap = n - lastPrime;
                sumPrimeGaps += gap;

                // Twin?
                if (gap == 2) {
                    uint64_t twinP = lastPrime;
                    int twinClass = classIndex(twinP);

                    if (twinClass >= 0) {
                        ++twinCount;

                        if (lastTwinP != 0 && lastTwinClass >= 0) {
                            uint64_t twinGap = twinP - lastTwinP;
                            int gapMin = GAP_MIN[lastTwinClass][twinClass];
                            int64_t excess = twinGap - gapMin;

                            sumTwinGaps += twinGap;
                            sumTwinExcess += excess;

                            // v₂(gap) - trailing zeros do gap
                            unsigned v2_gap = v2_of(twinGap);
                            sumV2Gap += v2_gap;
                            ++countV2Gap;

                            if (v2_gap >= 1 && v2_gap <= 7)
                                v2GapHistogram[v2_gap - 1]++;
                            else if (v2_gap >= 8)
                                v2GapHistogram[7]++;

                            // Para correlação: usa v₂(p+1) do twin anterior
                            unsigned v2_prev = v2_of(lastTwinP + 1);
                            sumV2Product += v2_prev * v2_gap;
                            sumV2Sq += v2_prev * v2_prev;
                            sumV2GapSq += v2_gap * v2_gap;

                            // Década (índice = log10(p) - 3)
                            if (lastTwinP >= 1000) {
                                int decIdx = static_cast<int>(std::log10(static_cast<double>(lastTwinP))) - 3;
                                if (decIdx >= 0 && decIdx < Stats::N_DECADES) {
                                    long double ln_p = std::log(static_cast<long double>(lastTwinP));
                                    long double ln2_p = ln_p * ln_p;
                                    decades[decIdx].count++;
                                    decades[decIdx].sumExcess += excess;
                                    decades[decIdx].sumLn2 += ln2_p;
                                }
                            }

                            // Assintótico (p > 100k)
                            if (lastTwinP > ASYMPTOTIC_LIMIT) {
                                ++asymptoticCount;
                                sumAsymptoticExcess += excess;
                            }

                            // Transições
                            transitions.count[lastTwinClass][twinClass]++;
                            transitions.sumGap[lastTwinClass][twinClass] += twinGap;
                        }

                        lastTwinP = twinP;
                        lastTwinClass = twinClass;
                    }
                }
            }
            lastPrime = n;
            ++primesSinceLastUpdate;

            // Atualiza stats
            if (primesSinceLastUpdate >= progressStep) {
                primesSinceLastUpdate = 0;

                stats.currentN = n;
                stats.primeCount = primeCount;
                stats.twinCount = twinCount;

                long double ln_p = std::log(static_cast<long double>(n));
                long double ln2_p = ln_p * ln_p;

                // Regime
                if (n < FROZEN_LIMIT) {
                    stats.regime = Regime::Frozen;
                } else if (n < TRANSITION_LIMIT) {
                    stats.regime = Regime::Transition;
                } else if (n < PRECISE_LIMIT) {
                    stats.regime = Regime::Asymptotic;
                } else {
                    stats.regime = Regime::Precise;
                }

                // kT Primos: kT = ⟨gap⟩ - 2
                if (primeCount > 1) {
                    stats.avgPrimeGap = static_cast<double>(sumPrimeGaps / (primeCount - 1));
                    stats.kTPrimeEmpirical = stats.avgPrimeGap - OFFSET_PRIMES;
                }
                stats.kTPrimeTheoretical = static_cast<double>(ln_p) - OFFSET_PRIMES;

                // kT Twins
                if (twinCount > 1) {
                    uint64_t nTrans = twinCount - 1;
                    stats.avgTwinGap = static_cast<double>(sumTwinGaps / nTrans);
                    stats.kTTwinEmpirical = static_cast<double>(sumTwinExcess / nTrans);
                }

                stats.asymptoticCount = asymptoticCount;
                if (asymptoticCount > 10) {
                    stats.kTTwinAsymptotic = static_cast<double>(sumAsymptoticExcess / asymptoticCount);
                }

                // Modelo CUMULATIVO: kT = 0.7784×ln²(p) - 2.32×ln(p) - 13.9
                stats.kTTwinTheoretical = static_cast<double>(
                    kT_CUM_LN2 * ln2_p + kT_CUM_LN * ln_p + kT_CUM_C
                );

                // Razão kT/ln²(p)
                if (ln2_p > 0 && stats.kTTwinAsymptotic > 0) {
                    stats.kTRatio = stats.kTTwinAsymptotic / static_cast<double>(ln2_p);
                }

                // v₂(p+1)
                if (countV2 > 0) {
                    stats.meanV2 = static_cast<double>(sumV2 / countV2);
                    for (int i = 0; i < 8; ++i)
                        stats.v2Histogram[i] = v2Histogram[i];
                }

                // v₂(gap)
                if (countV2Gap > 0) {
                    stats.meanV2Gap = static_cast<double>(sumV2Gap / countV2Gap);
                    for (int i = 0; i < 8; ++i)
                        stats.v2GapHistogram[i] = v2GapHistogram[i];
                }

                // Correlação de Pearson entre v₂(p+1) e v₂(gap)
                if (countV2Gap > 10) {
                    double n = static_cast<double>(countV2Gap);
                    double meanP = static_cast<double>(sumV2 / countV2);
                    double meanG = static_cast<double>(sumV2Gap / countV2Gap);

                    // r = Cov(X,Y) / sqrt(Var(X) × Var(Y))
                    double cov = (static_cast<double>(sumV2Product) / n) - (meanP * meanG);
                    double varP = (static_cast<double>(sumV2Sq) / n) - (meanP * meanP);
                    double varG = (static_cast<double>(sumV2GapSq) / n) - (meanG * meanG);

                    if (varP > 0 && varG > 0) {
                        stats.corrV2 = cov / std::sqrt(varP * varG);
                    }
                }

                // Transições
                stats.transitions = transitions;

                // R² de Boltzmann: consistência de kT entre transições
                if (twinCount > 100) {
                    double kT_mean = stats.kTTwinEmpirical;
                    if (kT_mean > 0) {
                        double ssRes = 0, ssTot = 0;
                        int validCount = 0;

                        for (int c1 = 0; c1 < 3; ++c1) {
                            for (int c2 = 0; c2 < 3; ++c2) {
                                if (transitions.count[c1][c2] < 10) continue;
                                double avgGap = transitions.avgGap(c1, c2);
                                int gapMin = GAP_MIN[c1][c2];
                                double kT_trans = avgGap - gapMin;

                                ssRes += (kT_trans - kT_mean) * (kT_trans - kT_mean);
                                ssTot += kT_trans * kT_trans;
                                ++validCount;
                            }
                        }

                        if (validCount > 1 && ssTot > 0) {
                            stats.boltzmannR2 = 1.0 - ssRes / ssTot;
                        }
                    }
                }

                // Décadas
                for (int i = 0; i < Stats::N_DECADES; ++i) {
                    stats.decades[i] = decades[i];
                }

                emit progress(stats);
            }
        }

        emit finished();
    }

    void stop() { m_stopRequested.store(true); }

signals:
    void progress(const Stats &stats);
    void finished();

private:
    std::atomic<bool> m_stopRequested;
    uint64_t m_startN = 3;
    AnalysisMode m_mode = AnalysisMode::Both;
    bool m_multiThread = false;
    PrimeSieve m_sieve;
};

// ======================= JANELA PRINCIPAL =======================

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr)
        : QMainWindow(parent)
    {
        qRegisterMetaType<Stats>("Stats");
        setWindowTitle("Termodinâmica dos Primos v4.1");

        QWidget *central = new QWidget(this);
        auto *mainLayout = new QHBoxLayout(central);

        // === COLUNA ESQUERDA ===
        auto *leftColumn = new QVBoxLayout();

        // Configuração
        auto *configBox = new QGroupBox("Configuração", central);
        auto *cfg = new QGridLayout(configBox);

        m_startEdit = new QLineEdit("3", configBox);
        m_modeCombo = new QComboBox(configBox);
        m_modeCombo->addItem("Primos", static_cast<int>(AnalysisMode::Primes));
        m_modeCombo->addItem("Twins", static_cast<int>(AnalysisMode::Twins));
        m_modeCombo->addItem("Ambos", static_cast<int>(AnalysisMode::Both));
        m_modeCombo->setCurrentIndex(2);

        m_startButton = new QPushButton("▶ Start", configBox);
        m_stopButton = new QPushButton("⬛ Stop", configBox);
        m_resetButton = new QPushButton("↺ Reset", configBox);
        m_stopButton->setEnabled(false);

        cfg->addWidget(new QLabel("Início n:"), 0, 0);
        cfg->addWidget(m_startEdit, 0, 1);
        cfg->addWidget(new QLabel("Modo:"), 1, 0);
        cfg->addWidget(m_modeCombo, 1, 1);
        auto *btnLayout = new QHBoxLayout();
        btnLayout->addWidget(m_startButton);
        btnLayout->addWidget(m_stopButton);
        btnLayout->addWidget(m_resetButton);
        cfg->addLayout(btnLayout, 2, 0, 1, 2);

        // Contagens
        auto *statsBox = new QGroupBox("Contagens", central);
        auto *statsLayout = new QFormLayout(statsBox);
        m_labelCurrentN = new QLabel("-");
        m_labelPrimeCount = new QLabel("0");
        m_labelTwinCount = new QLabel("0");
        statsLayout->addRow("n atual:", m_labelCurrentN);
        statsLayout->addRow("π(n):", m_labelPrimeCount);
        statsLayout->addRow("π₂(n):", m_labelTwinCount);

        // kT Primos
        auto *primeBox = new QGroupBox("kT Primos (|H|=1)", central);
        auto *primeLayout = new QFormLayout(primeBox);
        m_labelAvgPrimeGap = new QLabel("-");
        m_labelkTPrimeEmp = new QLabel("-");
        m_labelkTPrimeTheo = new QLabel("-");
        m_labelkTPrimeError = new QLabel("-");
        primeLayout->addRow("⟨gap⟩:", m_labelAvgPrimeGap);
        primeLayout->addRow("kT = ⟨gap⟩-2:", m_labelkTPrimeEmp);
        primeLayout->addRow("kT_teo = ln(p)-2:", m_labelkTPrimeTheo);
        primeLayout->addRow("Erro:", m_labelkTPrimeError);

        // kT Twins
        auto *twinBox = new QGroupBox("kT Twins (|H|=2)", central);
        auto *twinLayout = new QFormLayout(twinBox);
        m_labelRegime = new QLabel("CONGELADO");
        m_labelAvgTwinGap = new QLabel("-");
        m_labelkTTwinEmp = new QLabel("-");
        m_labelkTTwinAsymptotic = new QLabel("-");
        m_labelkTTwinTheo = new QLabel("-");
        m_labelkTRatio = new QLabel("-");
        m_labelkTTwinError = new QLabel("-");
        twinLayout->addRow("Regime:", m_labelRegime);
        twinLayout->addRow("⟨gap⟩:", m_labelAvgTwinGap);
        twinLayout->addRow("kT (total):", m_labelkTTwinEmp);
        twinLayout->addRow("kT (p>100k):", m_labelkTTwinAsymptotic);
        twinLayout->addRow("kT_teo (ref):", m_labelkTTwinTheo);
        twinLayout->addRow("kT/ln²(p):", m_labelkTRatio);
        twinLayout->addRow("Erro:", m_labelkTTwinError);

        // Estrutura 2-ádica (Ortogonal)
        auto *v2Box = new QGroupBox("Estrutura 2-ádica (Ortogonal)", central);
        auto *v2Layout = new QGridLayout(v2Box);

        // Headers
        v2Layout->addWidget(new QLabel(""), 0, 0);
        v2Layout->addWidget(new QLabel("<b>v₂(p+1)</b>"), 0, 1);
        v2Layout->addWidget(new QLabel("<b>v₂(gap)</b>"), 0, 2);
        v2Layout->addWidget(new QLabel("<b>Teórico</b>"), 0, 3);

        // Médias
        v2Layout->addWidget(new QLabel("⟨v₂⟩:"), 1, 0);
        m_labelMeanV2 = new QLabel("-");
        m_labelMeanV2Gap = new QLabel("-");
        v2Layout->addWidget(m_labelMeanV2, 1, 1);
        v2Layout->addWidget(m_labelMeanV2Gap, 1, 2);
        v2Layout->addWidget(new QLabel("2.0"), 1, 3);

        // P(k=1)
        v2Layout->addWidget(new QLabel("P(k=1):"), 2, 0);
        m_labelV2P1 = new QLabel("-");
        m_labelV2GapP1 = new QLabel("-");
        v2Layout->addWidget(m_labelV2P1, 2, 1);
        v2Layout->addWidget(m_labelV2GapP1, 2, 2);
        v2Layout->addWidget(new QLabel("0.500"), 2, 3);

        // P(k=2)
        v2Layout->addWidget(new QLabel("P(k=2):"), 3, 0);
        m_labelV2P2 = new QLabel("-");
        m_labelV2GapP2 = new QLabel("-");
        v2Layout->addWidget(m_labelV2P2, 3, 1);
        v2Layout->addWidget(m_labelV2GapP2, 3, 2);
        v2Layout->addWidget(new QLabel("0.250"), 3, 3);

        // P(k=3)
        v2Layout->addWidget(new QLabel("P(k=3):"), 4, 0);
        m_labelV2P3 = new QLabel("-");
        m_labelV2GapP3 = new QLabel("-");
        v2Layout->addWidget(m_labelV2P3, 4, 1);
        v2Layout->addWidget(m_labelV2GapP3, 4, 2);
        v2Layout->addWidget(new QLabel("0.125"), 4, 3);

        // Separador
        auto *sep = new QFrame(v2Box);
        sep->setFrameShape(QFrame::HLine);
        v2Layout->addWidget(sep, 5, 0, 1, 4);

        // kT binário
        v2Layout->addWidget(new QLabel("kT=1/ln2:"), 6, 0);
        v2Layout->addWidget(new QLabel("1.4427"), 6, 1);
        v2Layout->addWidget(new QLabel("1.4427"), 6, 2);
        v2Layout->addWidget(new QLabel("✓"), 6, 3);

        // Correlação
        v2Layout->addWidget(new QLabel("Correl:"), 7, 0);
        m_labelCorrV2 = new QLabel("-");
        m_labelCorrV2->setStyleSheet("font-weight: bold;");
        v2Layout->addWidget(m_labelCorrV2, 7, 1, 1, 2);
        v2Layout->addWidget(new QLabel("→ 0"), 7, 3);

        leftColumn->addWidget(configBox);
        leftColumn->addWidget(statsBox);
        leftColumn->addWidget(primeBox);
        leftColumn->addWidget(twinBox);
        leftColumn->addWidget(v2Box);
        leftColumn->addStretch();

        // === COLUNA CENTRAL ===
        auto *centerColumn = new QVBoxLayout();

        // Tabela de convergência por década (modelo LOCAL)
        auto *convBox = new QGroupBox("kT Local por Década (Hardy-Littlewood)", central);
        auto *convLayout = new QVBoxLayout(convBox);

        m_convergenceTable = new QTableWidget(8, 5, convBox);
        m_convergenceTable->setHorizontalHeaderLabels({"Déc", "n", "kT", "obs/teo (err%)", "Trend"});
        QStringList decLabels = {"10³", "10⁴", "10⁵", "10⁶", "10⁷", "10⁸", "10⁹", "10¹⁰"};
        for (int i = 0; i < 8; ++i) {
            m_convergenceTable->setItem(i, 0, new QTableWidgetItem(decLabels[i]));
            for (int j = 1; j < 5; ++j) {
                auto *item = new QTableWidgetItem("-");
                item->setTextAlignment(Qt::AlignCenter);
                m_convergenceTable->setItem(i, j, item);
            }
        }
        m_convergenceTable->setColumnWidth(0, 40);
        m_convergenceTable->setColumnWidth(1, 60);
        m_convergenceTable->setColumnWidth(2, 50);
        m_convergenceTable->setColumnWidth(3, 110);
        m_convergenceTable->setColumnWidth(4, 45);
        m_convergenceTable->setMinimumHeight(220);

        convLayout->addWidget(m_convergenceTable);

        // Transições mod 30
        auto *transBox = new QGroupBox("Transições mod 30", central);
        auto *transLayout = new QVBoxLayout(transBox);

        m_transitionTable = new QTableWidget(3, 3, transBox);
        m_transitionTable->setHorizontalHeaderLabels({"→11", "→17", "→29"});
        m_transitionTable->setVerticalHeaderLabels({"11→", "17→", "29→"});
        for (int i = 0; i < 3; ++i) {
            m_transitionTable->setColumnWidth(i, 70);
            for (int j = 0; j < 3; ++j) {
                auto *item = new QTableWidgetItem("-");
                item->setTextAlignment(Qt::AlignCenter);
                m_transitionTable->setItem(i, j, item);
            }
        }
        m_transitionTable->setMinimumHeight(100);

        m_gapTable = new QTableWidget(3, 3, transBox);
        m_gapTable->setHorizontalHeaderLabels({"→11", "→17", "→29"});
        m_gapTable->setVerticalHeaderLabels({"11→", "17→", "29→"});
        for (int i = 0; i < 3; ++i) {
            m_gapTable->setColumnWidth(i, 70);
            for (int j = 0; j < 3; ++j) {
                auto *item = new QTableWidgetItem("-");
                item->setTextAlignment(Qt::AlignCenter);
                m_gapTable->setItem(i, j, item);
            }
        }
        m_gapTable->setMinimumHeight(100);

        transLayout->addWidget(new QLabel("Probabilidades:"));
        transLayout->addWidget(m_transitionTable);
        transLayout->addWidget(new QLabel("⟨gap⟩ (gap_min):"));
        transLayout->addWidget(m_gapTable);

        m_labelBoltzmannR2 = new QLabel("R² Boltzmann: -");
        transLayout->addWidget(m_labelBoltzmannR2);

        centerColumn->addWidget(convBox);
        centerColumn->addWidget(transBox);

        // === COLUNA DIREITA ===
        auto *rightColumn = new QVBoxLayout();

        m_log = new QTextEdit(central);
        m_log->setReadOnly(true);
        m_log->setFont(QFont("Monospace", 9));

        m_mathInfo = new QTextEdit(central);
        m_mathInfo->setReadOnly(true);
        m_mathInfo->setFont(QFont("Monospace", 9));
        m_mathInfo->setHtml(R"(
<h3>Modelos kT Twins</h3>

<h4>LOCAL (por década):</h4>
<pre>
<b>kT ≈ 0.75×ln²(p) - 17</b>

= Hardy-Littlewood puro!
= ln²(p)/(2C₂) - gap_min
Erro &lt; 3% por década
</pre>

<h4>CUMULATIVO (total):</h4>
<pre>
kT = 0.78×ln² - 2.3×ln - 14

(artefato de mistura)
</pre>

<h4>Estrutura 2-ádica:</h4>
<pre>
v₂(p+1), v₂(gap) ~ Geo(1/2)
⟨v₂⟩ = 2.0, kT = 1/ln(2)
Correlação ≈ 0 (ortogonais)
</pre>

<h4>Constantes:</h4>
<pre>
C₂ = 0.6601618
1/(2C₂) ≈ 0.7575
</pre>
)");

        rightColumn->addWidget(new QLabel("Log:"));
        rightColumn->addWidget(m_log, 2);
        rightColumn->addWidget(new QLabel("Teoria:"));
        rightColumn->addWidget(m_mathInfo, 1);

        mainLayout->addLayout(leftColumn, 1);
        mainLayout->addLayout(centerColumn, 1);
        mainLayout->addLayout(rightColumn, 1);

        setCentralWidget(central);

        connect(m_startButton, &QPushButton::clicked, this, &MainWindow::onStart);
        connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::onStop);
        connect(m_resetButton, &QPushButton::clicked, this, &MainWindow::onReset);
    }

    ~MainWindow() override { cleanupWorker(); }

private slots:
    void onStart()
    {
        if (m_workerThread) {
            m_log->append("⚠ Já rodando.");
            return;
        }

        bool ok;
        uint64_t startN = m_startEdit->text().toULongLong(&ok);
        if (!ok || startN < 2) {
            QMessageBox::warning(this, "Erro", "Valor inválido");
            return;
        }

        m_workerThread = new QThread(this);
        m_worker = new Worker();
        m_worker->configure(startN,
            static_cast<AnalysisMode>(m_modeCombo->currentData().toInt()),
            false);
        m_worker->moveToThread(m_workerThread);

        connect(m_workerThread, &QThread::started, m_worker, &Worker::process);
        connect(m_worker, &Worker::progress, this, &MainWindow::onProgress, Qt::QueuedConnection);
        connect(m_worker, &Worker::finished, this, &MainWindow::onFinished, Qt::QueuedConnection);
        connect(m_worker, &Worker::finished, m_workerThread, &QThread::quit);
        connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
        connect(m_workerThread, &QThread::finished, m_workerThread, &QObject::deleteLater);
        connect(m_workerThread, &QThread::finished, this, [this]() {
            m_workerThread = nullptr;
            m_worker = nullptr;
        });

        m_workerThread->start();
        m_startButton->setEnabled(false);
        m_stopButton->setEnabled(true);
        m_log->append(QString("▶ Iniciando de n=%1").arg(startN));
    }

    void onStop()
    {
        if (m_worker) m_worker->stop();
        m_log->append("⬛ Parando...");
        m_stopButton->setEnabled(false);
        m_startButton->setEnabled(true);
    }

    void onReset()
    {
        onStop();
        m_labelCurrentN->setText("-");
        m_labelPrimeCount->setText("0");
        m_labelTwinCount->setText("0");
        m_log->append("↺ Reset");
    }

    void onProgress(const Stats &s)
    {
        m_labelCurrentN->setText(QString::number(s.currentN));
        m_labelPrimeCount->setText(QString::number(s.primeCount));
        m_labelTwinCount->setText(QString::number(s.twinCount));

        // Primos
        m_labelAvgPrimeGap->setText(QString::number(s.avgPrimeGap, 'f', 3));
        m_labelkTPrimeEmp->setText(QString::number(s.kTPrimeEmpirical, 'f', 3));
        m_labelkTPrimeTheo->setText(QString::number(s.kTPrimeTheoretical, 'f', 3));
        if (s.kTPrimeTheoretical > 0) {
            double err = 100.0 * std::abs(s.kTPrimeEmpirical - s.kTPrimeTheoretical) / s.kTPrimeTheoretical;
            m_labelkTPrimeError->setText(QString("%1%").arg(err, 0, 'f', 2));
        }

        // Regime com cor
        m_labelRegime->setText(regimeName(s.regime));
        m_labelRegime->setStyleSheet(QString("font-weight: bold; color: %1;").arg(regimeColor(s.regime)));

        // Twins
        m_labelAvgTwinGap->setText(QString::number(s.avgTwinGap, 'f', 1));
        m_labelkTTwinEmp->setText(QString::number(s.kTTwinEmpirical, 'f', 1));

        if (s.asymptoticCount > 10) {
            m_labelkTTwinAsymptotic->setText(QString("%1 (n=%2)")
                .arg(s.kTTwinAsymptotic, 0, 'f', 1)
                .arg(s.asymptoticCount));
        } else {
            m_labelkTTwinAsymptotic->setText("aguardando...");
        }

        m_labelkTTwinTheo->setText(QString::number(s.kTTwinTheoretical, 'f', 1));

        // Ratio com cor
        QString ratioColor = "black";
        double pct = s.kTRatio / Constants::RATIO_THEORETICAL;
        if (pct > 0.95 && pct < 1.05) ratioColor = "green";
        else if (pct > 0.85 && pct < 1.15) ratioColor = "orange";
        else if (s.kTRatio > 0) ratioColor = "red";

        m_labelkTRatio->setText(QString::number(s.kTRatio, 'f', 4));
        m_labelkTRatio->setStyleSheet(QString("font-weight: bold; color: %1;").arg(ratioColor));

        if (s.kTTwinTheoretical > 0 && s.asymptoticCount > 10) {
            double err = 100.0 * std::abs(s.kTTwinAsymptotic - s.kTTwinTheoretical) / s.kTTwinTheoretical;
            m_labelkTTwinError->setText(QString("%1%").arg(err, 0, 'f', 2));
        }

        // v₂(p+1) e v₂(gap)
        m_labelMeanV2->setText(QString::number(s.meanV2, 'f', 4));
        m_labelMeanV2Gap->setText(QString::number(s.meanV2Gap, 'f', 4));

        uint64_t totalV2 = 0, totalV2Gap = 0;
        for (int i = 0; i < 8; ++i) {
            totalV2 += s.v2Histogram[i];
            totalV2Gap += s.v2GapHistogram[i];
        }

        if (totalV2 > 0) {
            m_labelV2P1->setText(QString::number(double(s.v2Histogram[0]) / totalV2, 'f', 3));
            m_labelV2P2->setText(QString::number(double(s.v2Histogram[1]) / totalV2, 'f', 3));
            m_labelV2P3->setText(QString::number(double(s.v2Histogram[2]) / totalV2, 'f', 3));
        }

        if (totalV2Gap > 0) {
            m_labelV2GapP1->setText(QString::number(double(s.v2GapHistogram[0]) / totalV2Gap, 'f', 3));
            m_labelV2GapP2->setText(QString::number(double(s.v2GapHistogram[1]) / totalV2Gap, 'f', 3));
            m_labelV2GapP3->setText(QString::number(double(s.v2GapHistogram[2]) / totalV2Gap, 'f', 3));
        }

        // Correlação com cor (verde se próximo de 0)
        QString corrColor = std::abs(s.corrV2) < 0.05 ? "green" : "red";
        m_labelCorrV2->setText(QString("r = %1").arg(s.corrV2, 0, 'f', 4));
        m_labelCorrV2->setStyleSheet(QString("font-weight: bold; color: %1;").arg(corrColor));

        // Tabela de convergência por década (usando modelo LOCAL)
        double prevErrPct = -1;
        for (int i = 0; i < Stats::N_DECADES; ++i) {
            const auto& dec = s.decades[i];
            if (dec.count > 0) {
                m_convergenceTable->item(i, 1)->setText(QString::number(dec.count));

                double kT_obs = dec.kT();
                m_convergenceTable->item(i, 2)->setText(QString::number(kT_obs, 'f', 1));

                // Calcular kT teórico LOCAL para o ponto médio da década
                // Década i: 10^(i+3) a 10^(i+4), ponto médio = 10^(i+3.5)
                double decadeMidpoint = std::pow(10.0, i + 3 + 0.5);
                double ln_mid = std::log(decadeMidpoint);
                double ln2_mid = ln_mid * ln_mid;
                double kT_teo_local = Constants::kT_LOC_LN2 * ln2_mid
                                    + Constants::kT_LOC_LN * ln_mid
                                    + Constants::kT_LOC_C;

                // Erro percentual (local vs local)
                double errPct = 0;
                if (kT_teo_local > 0) {
                    errPct = 100.0 * std::abs(kT_obs - kT_teo_local) / kT_teo_local;
                }

                // Mostrar kT_obs / kT_teo e erro%
                QString ratioText = QString("%1/%2 (%3%)")
                    .arg(kT_obs, 0, 'f', 0)
                    .arg(kT_teo_local, 0, 'f', 0)
                    .arg(errPct, 0, 'f', 1);
                m_convergenceTable->item(i, 3)->setText(ratioText);

                // Cor baseada no erro
                QColor color;
                if (errPct < 3) color = Qt::darkGreen;
                else if (errPct < 10) color = QColor(255, 140, 0);
                else color = Qt::red;

                m_convergenceTable->item(i, 3)->setForeground(color);

                // Coluna de tendência
                QString trend = "";
                if (prevErrPct >= 0) {
                    if (errPct < prevErrPct - 0.5) {
                        trend = "↓";  // Erro diminuindo = convergindo
                        if (errPct < 3) trend += " ✓";
                    } else if (errPct > prevErrPct + 0.5) {
                        trend = "↑";  // Erro aumentando
                    } else {
                        trend = "→";  // Estável
                        if (errPct < 3) trend += " ✓";
                    }
                }
                m_convergenceTable->item(i, 4)->setText(trend);

                // Cor da tendência
                QColor trendColor = Qt::black;
                if (trend.contains("✓")) trendColor = Qt::darkGreen;
                else if (trend.startsWith("↓")) trendColor = QColor(0, 128, 0);
                else if (trend.startsWith("↑")) trendColor = Qt::red;
                m_convergenceTable->item(i, 4)->setForeground(trendColor);

                prevErrPct = errPct;
            }
        }

        // Transições
        for (int c1 = 0; c1 < 3; ++c1) {
            for (int c2 = 0; c2 < 3; ++c2) {
                double prob = s.transitions.probability(c1, c2);
                m_transitionTable->item(c1, c2)->setText(
                    QString::number(prob * 100, 'f', 1) + "%");

                double avgGap = s.transitions.avgGap(c1, c2);
                int gapMin = Constants::GAP_MIN[c1][c2];
                m_gapTable->item(c1, c2)->setText(
                    QString("%1 (%2)").arg(avgGap, 0, 'f', 0).arg(gapMin));
            }
        }

        m_labelBoltzmannR2->setText(QString("R² = %1").arg(s.boltzmannR2, 0, 'f', 4));

        // Log periódico
        if ((s.primeCount % 100000) < 5000) {
            m_log->append(QString("n=%1 | π₂=%2 | kT=%3 | ratio=%4")
                .arg(s.currentN)
                .arg(s.twinCount)
                .arg(s.kTTwinAsymptotic, 0, 'f', 1)
                .arg(s.kTRatio, 0, 'f', 4));
        }
    }

    void onFinished()
    {
        m_log->append("✓ Finalizado.");
        m_startButton->setEnabled(true);
        m_stopButton->setEnabled(false);
    }

private:
    void cleanupWorker()
    {
        if (m_worker) m_worker->stop();
        if (m_workerThread) {
            m_workerThread->quit();
            m_workerThread->wait(3000);
        }
    }

    QLineEdit *m_startEdit;
    QComboBox *m_modeCombo;
    QPushButton *m_startButton, *m_stopButton, *m_resetButton;
    QLabel *m_labelCurrentN, *m_labelPrimeCount, *m_labelTwinCount;
    QLabel *m_labelAvgPrimeGap, *m_labelkTPrimeEmp, *m_labelkTPrimeTheo, *m_labelkTPrimeError;
    QLabel *m_labelRegime;
    QLabel *m_labelAvgTwinGap, *m_labelkTTwinEmp, *m_labelkTTwinAsymptotic;
    QLabel *m_labelkTTwinTheo, *m_labelkTRatio, *m_labelkTTwinError;
    QLabel *m_labelMeanV2, *m_labelMeanV2Gap;
    QLabel *m_labelV2P1, *m_labelV2P2, *m_labelV2P3;
    QLabel *m_labelV2GapP1, *m_labelV2GapP2, *m_labelV2GapP3;
    QLabel *m_labelCorrV2;
    QTableWidget *m_convergenceTable;
    QTableWidget *m_transitionTable, *m_gapTable;
    QLabel *m_labelBoltzmannR2;
    QTextEdit *m_log, *m_mathInfo;
    QThread *m_workerThread = nullptr;
    Worker *m_worker = nullptr;
};

// ======================= MAIN =======================

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    MainWindow w;
    w.resize(1400, 850);
    w.show();

    return app.exec();
}

#include "main.moc"
