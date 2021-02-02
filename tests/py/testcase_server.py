#! /usr/bin/python3

import unittest
import module.shmpy as shmpy
import os


class TestServer(unittest.TestCase):

    def setUp(self) -> None:
        self.svr = shmpy.server('test')
        self.assertEqual(self.svr.name, "test")
        self.assertEqual(self.svr.ref_count, 1)
        self.assertEqual(self.svr.id, 0)
        self.assertEqual(self.svr.owner_pid, os.getpid())
        self.assertEqual(self.svr.status, shmpy.ok)
        self.assertEqual(len(self.svr.clients_id), 0)

    def tearDown(self) -> None:
        del self.svr
        return super().tearDown()

    def test_insertInt(self) -> None:
        self.svr.int_insert('int1', 100)
        self.assertEqual(self.svr.int_get('int1'), 100)

        self.svr.int_insert('int2', -100)
        self.assertEqual(self.svr.int_get('int2'), -100)

        self.svr.int_insert('int3', 0)
        self.assertEqual(self.svr.int_get('int3'), 0)

        self.svr.int_insert('int4', 1 << 31)
        self.assertEqual(self.svr.int_get('int4'), 1 << 31)

    def test_setInt(self) -> None:
        self.svr.int_insert('int1', 100)
        self.svr.int_set('int1', 200)
        self.assertEqual(self.svr.int_get('int1'), 200)

        self.svr.int_insert('int2', 100)
        self.svr.int_set('int2', -100)
        self.assertEqual(self.svr.int_get('int2'), -100)

        self.svr.int_insert('int3', 500)
        self.svr.int_set('int3', 0)
        self.assertEqual(self.svr.int_get('int3'), 0)

    def test_insertFloat(self) -> None:
        self.svr.float_insert('float1', 3.14)
        self.assertAlmostEqual(self.svr.float_get('float1'), 3.14)

        self.svr.float_insert('float2', 3.140932842123)
        self.assertAlmostEqual(self.svr.float_get('float2'), 3.140932842123)

    def test_setFloat(self) -> None:
        self.svr.float_insert('float1', 3.14)
        self.svr.float_set('float1', 1231.123123)
        self.assertAlmostEqual(self.svr.float_get('float1'), 1231.123123)

        self.svr.float_insert('float2', 3.140932842123)
        self.svr.float_set('float2', 1231.123123)
        self.assertAlmostEqual(self.svr.float_get('float2'), 1231.123123)

    def test_insertBool(self) -> None:
        self.svr.bool_insert('bool1', True)
        self.assertTrue(self.svr.bool_get('bool1'))

        self.svr.bool_insert('bool2', False)
        self.assertFalse(self.svr.bool_get('bool2'))

    def test_setBool(self) -> None:
        self.svr.bool_insert('bool1', True)
        self.svr.bool_set('bool1', False)
        self.assertFalse(self.svr.bool_get('bool1'))

        self.svr.bool_insert('bool2', False)
        self.svr.bool_set('bool2', True)
        self.assertTrue(self.svr.bool_get('bool2'))

    def test_insertStr(self) -> None:
        self.svr.str_insert('str1', "Hello World")
        self.assertEqual(self.svr.str_get('str1'), "Hello World")

        self.svr.str_insert('str2', "今天天气好晴朗!")
        self.assertEqual(self.svr.str_get('str2'), "今天天气好晴朗!")

    def test_setStr(self) -> None:
        self.svr.str_insert('str1', "Hello World")
        self.svr.str_set('str1', "Rush B!")
        self.assertEqual(self.svr.str_get('str1'), "Rush B!")
        self.svr.str_set('str1', "Rush B!lkjdsa;lkfjlsajf;")
        self.assertEqual(self.svr.str_get('str1'),
                         "Rush B!lkjdsa;lkfjlsajf;")

        self.svr.str_insert('str2', "今天天气好晴朗!")
        self.svr.str_set('str2', "我佛了!今天天气好晴朗")
        self.assertEqual(self.svr.str_get('str2'), "我佛了!今天天气好晴朗")
        self.svr.str_set('str2', "我佛了!")
        self.assertEqual(self.svr.str_get('str2'), "我佛了!")


if __name__ == '__main__':
    unittest.main()
