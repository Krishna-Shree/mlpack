/**
 * @file lars_main.cpp
 * @author Nishant Mehta
 *
 * Executable for LARS
 */
#include <mlpack/core.hpp>

#include "lars.hpp"

PROGRAM_INFO("LARS", "An implementation of LARS: Least Angle Regression "
    "(Stagewise/laSso).  This is a stage-wise homotopy-based algorithm for "
    "L1-regularized linear regression (LASSO) and L1+L2-regularized linear "
    "regression (Elastic Net).\n"
    "\n"
    "Let X be a matrix where each row is a point and each column is a "
    "dimension, and let y be a vector of targets.\n"
    "\n"
    "The Elastic Net problem is to solve\n\n"
    "  min_beta || X * beta - y ||_2^2 + lambda_1 ||beta||_1 +\n"
    "      0.5 lambda_2 ||beta||_2^2\n\n"
    "If lambda_1 > 0 and lambda_2 = 0, the problem is the LASSO.\n"
    "If lambda_1 > 0 and lambda_2 > 0, the problem is the Elastic Net.\n"
    "If lambda_1 = 0 and lambda_2 > 0, the problem is Ridge Regression.\n"
    "If lambda_1 = 0 and lambda_2 = 0, the problem is unregularized linear "
    "regression.\n"
    "\n"
    "For efficiency reasons, it is not recommended to use this algorithm with "
    "lambda_1 = 0.\n"
    "\n"
    "For more details, see the following papers:\n"
    "\n"
    "@article{\n"
    "  title = {Least angle regression},\n"
    "  author = {Efron, B. and Hastie, T. and Johnstone, I. and Tibshirani,"
    "R.},\n"
    "  journal = {The Annals of Statistics},\n"
    "  volume = {32},\n"
    "  number = {2},\n"
    "  pages = {407--499},\n"
    "  year = {2004},\n"
    "  publisher = {Institute of Mathematical Statistics}\n"
    "}\n"
    "\n"
    "@article{\n"
    "  title = {Regularization and variable selection via the elastic net},\n"
    "  author = {Zou, H. and Hastie, T.},\n"
    "  journal = {Journal of the Royal Statistical Society Series B},\n"
    "  volume = {67},\n"
    "  number = {2},\n"
    "  pages = {301--320},\n"
    "  year = {2005},\n"
    "  publisher = {Royal Statistical Society}\n"
    "}");

PARAM_STRING_REQ("input_file", "File containing covariates (X)",
    "i");
PARAM_STRING_REQ("responses_file", "File containing y "
    "(responses/observations).", "r");

PARAM_STRING("output_file", "File to save beta (linear estimator) to", "o",
    "output.csv");

PARAM_DOUBLE("lambda1", "Regularization parameter for l1-norm penalty", "", 0);
PARAM_DOUBLE("lambda2", "Regularization parameter for l2-norm penalty", "", 0);
PARAM_FLAG("use_cholesky", "Use Cholesky decomposition during computation "
    "rather than explicitly computing the full Gram matrix", "");

using namespace arma;
using namespace std;
using namespace mlpack;
using namespace mlpack::regression;

int main(int argc, char* argv[])
{
  // Handle parameters,
  CLI::ParseCommandLine(argc, argv);

  double lambda1 = CLI::GetParam<double>("lambda1");
  double lambda2 = CLI::GetParam<double>("lambda2");
  bool useCholesky = CLI::GetParam<bool>("use_cholesky");

  // Load covariates.
  const string matXFilename = CLI::GetParam<string>("input_file");
  mat matX;
  data::Load(matXFilename.c_str(), matX, true);

  // Load targets.
  const string yFilename = CLI::GetParam<string>("responses_file");
  mat matY; // Will be a vector.
  data::Load(yFilename.c_str(), matY, true);

  // Make sure y is oriented the right way.
  if (matY.n_rows == 1)
    matY = trans(matY);
  if (matY.n_cols > 1)
    Log::Fatal << "Only one column or row allowed in responses file!" << endl;

  if (matY.n_elem != matX.n_cols)
    Log::Fatal << "Number of responses must be equal to number of rows of X!"
        << endl;

  // Do LARS.
  LARS lars(useCholesky, lambda1, lambda2);
  lars.DoLARS(trans(matX), matY.unsafe_col(0));

  // Get and save solution.
  vec beta;
  lars.Solution(beta);

  const string betaFilename = CLI::GetParam<string>("output_file");
  beta.save(betaFilename, raw_ascii);
}