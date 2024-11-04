#ifndef HELPERFUNCTIONS_H_
#define HELPERFUNCTIONS_H_

class HelperFunctions
{
public:
    /**
     * Find free TCP port
     *
     * @return int next free TCP port
     */
    static int getFreePort();

    /**
     * @brief Set pipe error flag
     */
    static void setPipeError();

    /**
     * @brief Get and reset pipe error flag
     *
     * @return bool false if pipe error flag is not set
     */
    static bool getAndResetPipeError();

private:
    static bool pipeError;
};

#endif // HELPERFUNCTIONS_H_
