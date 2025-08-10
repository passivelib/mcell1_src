#include <string>

namespace mlic
{

    enum class FeatureType
    {
        MCELL = 1,
        MDOC
    };

    class License
    {
    private:
        bool vl; // valid license

    public:
        License(std::string envVar, double version, const mlic::FeatureType &featureType);

        bool getVL() const; // valid license
    };
}