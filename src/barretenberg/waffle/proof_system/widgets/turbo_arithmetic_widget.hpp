#pragma once

#include "./base_widget.hpp"

namespace waffle {
class VerifierTurboArithmeticWidget : public VerifierBaseWidget {
  public:
    VerifierTurboArithmeticWidget(std::vector<barretenberg::g1::affine_element>& instance_commitments);

    VerifierBaseWidget::challenge_coefficients append_scalar_multiplication_inputs(
        const challenge_coefficients& challenge,
        const transcript::Transcript& transcript,
        std::vector<barretenberg::g1::affine_element>& points,
        std::vector<barretenberg::fr::field_t>& scalars);


    barretenberg::fr::field_t compute_batch_evaluation_contribution(barretenberg::fr::field_t& batch_eval,
                                                                    const barretenberg::fr::field_t& nu_base,
                                                                    const transcript::Transcript& transcript);


    barretenberg::fr::field_t compute_quotient_evaluation_contribution(const barretenberg::fr::field_t& alpha_base, const transcript::Transcript& transcript, barretenberg::fr::field_t&)
    {
        barretenberg::fr::field_t alpha = barretenberg::fr::serialize_from_buffer(transcript.get_challenge("alpha").begin());
        return barretenberg::fr::mul(alpha_base, alpha);
    }
};

class ProverTurboArithmeticWidget : public ProverBaseWidget {
  public:
    ProverTurboArithmeticWidget(const size_t n);
    ProverTurboArithmeticWidget(const ProverTurboArithmeticWidget& other);
    ProverTurboArithmeticWidget(ProverTurboArithmeticWidget&& other);
    ProverTurboArithmeticWidget& operator=(const ProverTurboArithmeticWidget& other);
    ProverTurboArithmeticWidget& operator=(ProverTurboArithmeticWidget&& other);

    barretenberg::fr::field_t compute_quotient_contribution(const barretenberg::fr::field_t& alpha_base,
                                                            const transcript::Transcript& transcript,
                                                            CircuitFFTState& circuit_state);
    barretenberg::fr::field_t compute_linear_contribution(const barretenberg::fr::field_t& alpha_base,
                                                          const transcript::Transcript& transcript,
                                                          const barretenberg::evaluation_domain& domain,
                                                          barretenberg::polynomial& r);
    barretenberg::fr::field_t compute_opening_poly_contribution(const barretenberg::fr::field_t& nu_base,
                                                                const transcript::Transcript&,
                                                                barretenberg::fr::field_t*,
                                                                barretenberg::fr::field_t*,
                                                                const barretenberg::evaluation_domain&);

    std::unique_ptr<VerifierBaseWidget> compute_preprocessed_commitments(const barretenberg::evaluation_domain& domain,
                                                                         const ReferenceString& reference_string) const;

    void compute_transcript_elements(transcript::Transcript& transcript, const barretenberg::evaluation_domain&);

    void reset(const barretenberg::evaluation_domain& domain);

    barretenberg::polynomial q_m;
    barretenberg::polynomial q_c;
    barretenberg::polynomial q_1;
    barretenberg::polynomial q_2;
    barretenberg::polynomial q_3;
    barretenberg::polynomial q_4;
    barretenberg::polynomial q_4_next;

    barretenberg::polynomial q_arith;

    barretenberg::polynomial q_1_fft;
    barretenberg::polynomial q_2_fft;
    barretenberg::polynomial q_3_fft;
    barretenberg::polynomial q_4_fft;
    barretenberg::polynomial q_4_next_fft;
    barretenberg::polynomial q_m_fft;
    barretenberg::polynomial q_c_fft;
    barretenberg::polynomial q_arith_fft;
};
} // namespace waffle