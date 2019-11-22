/*++

    Copyright (c) Microsoft Corporation.
    Licensed under the MIT License.

--*/

typedef struct _QUIC_LOSS_DETECTION {

    //
    // Number of outstanding **retransmittable** packets.
    //
    uint32_t PacketsInFlight;

    //
    // Largest acknowledged packet number.
    //
    uint64_t LargestAck;

    //
    // The highest encryption level we've received an ACK for.
    //
    QUIC_ENCRYPT_LEVEL LargestAckEncryptLevel;

    //
    // N.B.: SentPackets and LostPackets are generally kept in ascending packet
    // number order, and packets in the LostPackets list generally have smaller
    // numbers than those in the SentPackets list. The only case this is not
    // true is during the handshake. Since multiple encryption levels are used
    // in parallel, higher numbered packets in lower encryption levels can be
    // "lost" sooner than the higher encryption levels.
    //

    //
    // Outstanding packets.
    //
    uint64_t LargestSentPacketNumber;
    PQUIC_SENT_PACKET_METADATA SentPackets;
    PQUIC_SENT_PACKET_METADATA* SentPacketsTail;

    uint32_t TimeOfLastPacketSent;

    //
    // Lost packets. The purpose of this list is to remember packets a little
    // while after we decide they are lost, in case we were wrong and the ACK
    // comes in later than expected. For accounting purposes we don't consider
    // these packets to be in the network.
    //
    PQUIC_SENT_PACKET_METADATA LostPackets;
    PQUIC_SENT_PACKET_METADATA* LostPacketsTail;

    //
    // Number of probes sent.
    //
    uint16_t ProbeCount;

} QUIC_LOSS_DETECTION, *PQUIC_LOSS_DETECTION;

_IRQL_requires_max_(PASSIVE_LEVEL)
void
QuicLossDetectionInitialize(
    _Inout_ PQUIC_LOSS_DETECTION LossDetection
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
void
QuicLossDetectionUninitialize(
    _In_ PQUIC_LOSS_DETECTION LossDetection
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
void
QuicLossDetectionReset(
    _In_ PQUIC_LOSS_DETECTION LossDetection
    );

//
// Called when a particular key type has been discarded. This removes
// the tracking for all related outstanding packets.
//
_IRQL_requires_max_(PASSIVE_LEVEL)
void
QuicLossDetectionDiscardPackets(
    _In_ PQUIC_LOSS_DETECTION LossDetection,
    _In_ QUIC_PACKET_KEY_TYPE KeyType
    );

//
// Called when 0-RTT data was rejected.
//
_IRQL_requires_max_(PASSIVE_LEVEL)
void
QuicLossDetectionOnZeroRttRejected(
    _In_ PQUIC_LOSS_DETECTION LossDetection
    );

//
// Resets the timer based on the current state.
//
_IRQL_requires_max_(PASSIVE_LEVEL)
void
QuicLossDetectionUpdateTimer(
    _In_ PQUIC_LOSS_DETECTION LossDetection
    );

//
// Returns the current PTO in microseconds.
//
_IRQL_requires_max_(PASSIVE_LEVEL)
uint32_t
QuicLossDetectionComputeProbeTimeout(
    _In_ PQUIC_LOSS_DETECTION LossDetection,
    _In_ const QUIC_PATH* Path,
    _In_ uint32_t Count
    );

//
// Called when a new packet is sent.
//
_IRQL_requires_max_(PASSIVE_LEVEL)
QUIC_STATUS
QuicLossDetectionOnPacketSent(
    _In_ PQUIC_LOSS_DETECTION LossDetection,
    _In_ QUIC_PATH* Path,
    _In_ PQUIC_SENT_PACKET_METADATA SentPacket
    );

//
// Processes a received ACK frame. Returns true if the frame could be
// successfully processed. On failure, 'InvalidFrame' indicates if the frame
// was corrupt or not.
//
_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN
QuicLossDetectionProcessAckFrame(
    _In_ PQUIC_LOSS_DETECTION LossDetection,
    _In_ QUIC_PATH* Path,
    _In_ QUIC_ENCRYPT_LEVEL EncryptLevel,
    _In_ QUIC_FRAME_TYPE FrameType,
    _In_ uint16_t BufferLength,
    _In_reads_bytes_(BufferLength)
        const uint8_t* const Buffer,
    _Inout_ uint16_t* Offset,
    _Out_ BOOLEAN* InvalidFrame
    );

//
// Called when the loss detection timer fires.
//
_IRQL_requires_max_(PASSIVE_LEVEL)
void
QuicLossDetectionProcessTimerOperation(
    _In_ PQUIC_LOSS_DETECTION LossDetection
    );
