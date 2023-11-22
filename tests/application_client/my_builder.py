from tonsdk.boc import Builder


class MyBuilder(Builder):
    def store_maybe_ref(self, src):
        if src is not None:
            self.store_bit(1)
            self.store_ref(src)
        else:
            self.store_bit(0)

        return self

    def store_string_tail(self, bs: bytes):
        store_max = self.bits.get_free_bits() // 8
        self.store_bytes(bs[:store_max])
        if len(bs) > store_max:
            inner = begin_cell().store_string_tail(bs[store_max:]).end_cell()
            self.store_ref(inner)

        return self


def begin_cell():
    return MyBuilder()
