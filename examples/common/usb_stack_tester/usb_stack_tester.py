import usb.core

TRIAL = 3
TIMEOUT = 100 #milliseconds

def test_func(name, dev, wLength, wValue):
	wLength = int(wLength)
	wValue = int(wValue)
	print("\tNAME %s" % name)
	print("\t\tPARAM wLength = %i (requested)" % wLength)
	print("\t\tPARAM wValue = %i (returned)" % wValue)
	global TRAIL, TIMEOUT
	trial_num = TRIAL
	while trial_num:
		res = None
		try:
			buf = dev.ctrl_transfer(0xC0, 0,
				data_or_wLength=wLength, wValue=wValue,
				timeout=TIMEOUT)
			res = "SUCCESS (length = %i)" % len(buf)
		except:
			res = "FAILED"

		print("\t\ttrial(%i): %s" % (trial_num, res))
		trial_num -= 1
	print("")

if __name__ == "__main__":
	dev = usb.core.find(idVendor=0xcafe, idProduct=0xcafe)
	if dev is None:
		raise ValueError('Device not found')

	dev.set_configuration()

	ep0_size = dev.bMaxPacketSize0
	print("CONSTANTS")
	print("bMaxPacketSize0: %i" % ep0_size)
	print("TRIAL: %i" % TRIAL)
	print("TIMEOUT: %i ms" % TIMEOUT)

	print("1. device return the equal amount of data that the host is expecting.")
	print("1.1. the length of data is multiple of bMaxEndpointSize0")
	if True:
		test_func("variant-1", dev, ep0_size, ep0_size)
		test_func("variant-2", dev, ep0_size * 2, ep0_size * 2)
	print("1.2. the length of data is not multiple of bMaxEndpointSize0")
	if True:
		test_func("variant-1", dev, ep0_size + 3, ep0_size + 3)
		test_func("variant-2", dev, ep0_size * 1.5, ep0_size * 1.5)
		test_func("variant-3", dev, ep0_size * 0.5, ep0_size * 0.5)
		test_func("variant-4", dev, ep0_size - 10, ep0_size - 10)

	print("2. device return less data than the host is expecting")
	print("2.1. the length of data is returned multiple of bMaxEndpointSize0")
	if True:
		test_func("variant-1", dev, ep0_size * 2, ep0_size)
		test_func("variant-2", dev, ep0_size + 7, ep0_size)
		test_func("variant-3", dev, (ep0_size * 2) - 11, ep0_size)
	print("2.2 the length of data is not multiple of bMaxEndpointSize0")
	if True:
		test_func("variant-1", dev, 15, 7)
		test_func("variant-2", dev, ep0_size * 1.5, ep0_size + 1)
		test_func("variant-3", dev, ep0_size + 1, ep0_size - 1)
	print("2.3. wLength > 0, but device return 0 bytes, (similar to case 2.1)")
	if True:
		test_func("variant-1", dev, ep0_size, 0)
		test_func("variant-2", dev, 100, 0)

	print("3. wLength = 0, no data stage")
	if True:
		test_func("variant-1", dev, 0, 0)

