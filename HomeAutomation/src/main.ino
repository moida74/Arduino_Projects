/*
 * Nexa Receiver
 * Controls 4 relays connected to Aurduino I/O pins.
 * Transmitter codes are hard-coded as a fixed number in the Sketch.
 * src: http://www.telldus.com/forum/viewtopic.php?f=12&t=4072
 *
 * The code has been validated to work with the following NEXA transmitters:
 *
 * WBT 912 2ch sender
 * WT2 PRO 2ch Wall sender
 * LMDT 810 Wireless Outdoor motion sensor
 * LMST 606 Wireless Magnetic contact
 * PB3 kit sender
 * Telstick Net
 *
 * Please note that timing varies some between different senders.
 * The Debug Serial port can be used to identifiy Sender codes and also for debug timing issues.
 *
 * Receiver Hardware is a 1 USD 433MHz wireless reciever module bought on Ebay. Search for "433Mhz RF transmitter and receiver arduino" to find a suitable reciever.
 *
 * Receiver functionality is based on original code from
 * Barnaby Gray 12/2008
 * Peter Mead 09/2008
 * "Homeeasy protocol receiver for the new protocol."
 *
 * * The data is encoded on the wire (aerial) as a Manchester code.
 *
 * A latch of 275us high, 2675us low is sent before the data.
 * There is a gap of 10ms between each message.
 *
 * 0 = holding the line high for 275us then low for 275us.
 * 1 = holding the line high for 275us then low for 1225us.
 *
 * The timings seem to vary quite noticably between devices.
 * If this script does not detect your signals try relaxing the timing
 * conditions.
 * *
 * Each actual bit of data is encoded as two bits on the wire as:
 * Data 0 = Wire 01
 * Data 1 = Wire 10
 *
 * The actual message is 32 bits of data (64 wire bits):
 * bits 0-25: the group code - a 26bit number assigned to controllers.
 * bit 26: group flag
 * bit 27: on/off flag
 * bits 28-31: the device code - a 4bit number.
 *
 * The group flag just seems to be a separate set of addresses you can program devices
 * to and doesn't trigger the dim cycle when sending two ONs.
 */


int rxPin = D0; // Input of 433 MHz receiver


int t1 = 0; // Latch 1 time only needed for debugging purposes
int t2 = 0; //latch 2 time only needed for debugging purposes.

void setup()
{
  Serial.begin(115200);
  Serial.println ("Nexa Receiver Hard Coded");
  pinMode(rxPin, INPUT); // Input of 433 MHz receiver
  delay(1000);
}


void loop()
{
  int i = 0;
  int g = 0;
  unsigned long t = 0;

  byte prevBit = 0;
  byte bit = 0;

  unsigned long sender = 0;
  bool group = false;
  bool on = false;
  unsigned int recipient = 0;

  // latch 1
  // Latch timing has been loosened to acommodate varieties in the Nexa transmitters.


  while ((t < 8000 || t > 13000))
  { t = pulseIn(rxPin, LOW, 1000000);
    t1 = t; // Save latch timing for debugging purposes
  }

  // Next line can be used to debug latch timing. Please note that this affects timing and that recieving the following data bits may fail.
  // Serial.println (t);

  // latch 2
  // Latch timing has been loosened to acommodate varieties in the Nexa transmitters.
  while (t < 2200 || t > 2900)
  {
    t = pulseIn(rxPin, LOW, 1000000);
  }
  t2 = t; // Save latch timing for debugging purposes
  // Next line can be used to debug latch timing. Please note that this affects timing and that recieving the following data bits may fail.
  //Serial.println (t);

  // data collection from reciever circuit
  while (i < 64)
  {
    t = pulseIn(rxPin, LOW, 1000000);

    if (t > 200 && t < 400)
    { bit = 0;

    }
    else if (t > 1100 && t < 1560)
    { bit = 1;

    }
    else
    { i = 0;

      break;
    }

    if (i % 2 == 1)
    {
      if ((prevBit ^ bit) == 0)
      { // must be either 01 or 10, cannot be 00 or 11
        i = 0;
        break;
      }

      if (i < 53) { // first 26 data bits
        sender <<= 1;
        sender |= prevBit;
      }
      else if (i == 53)
      { // 26th data bit
        group = prevBit;
      }
      else if (i == 55)
      { // 27th data bit
        on = prevBit;
      }
      else
      { // last 4 data bits
        recipient <<= 1;
        recipient |= prevBit;
      }
    }

    prevBit = bit;
    ++i;
  }

  // interpret message
  if (i > 0) printResult(sender, group, on, recipient); // Print the result on Serial Monitor. Use this to identify your transmitter code.


}



void printResult(unsigned long sender, bool group, bool on, unsigned int recipient)
{
  Serial.print("sender ");
  Serial.print(sender);

  if (group)
  { Serial.print(" group ");
  }
  else
  { Serial.print(" !group ");
  }

  if (on)
  { Serial.print(" on");
  }
  else
  { Serial.print(" off");
  }

  Serial.print(" recipient ");
  Serial.println(recipient);
  Serial.print("Timings: ");
  Serial.print(t1); // Timing for latch 1
  Serial.print(",");
  Serial.println(t2); // Timing for latch 2

}
