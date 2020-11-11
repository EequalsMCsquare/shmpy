#include "libshmpy.hpp"
#include "data_server.hpp"
#include "data_client.hpp"
#include <gtest/gtest.h>


class TestCase_Pool : public testing::Test {

public:
    std::shared_ptr<Server> s;
    std::shared_ptr<Client> c1;

    virtual void SetUp() {
        this->s = std::make_shared<Server>("server1", 128);
        this->c1 = std::make_shared<Client>("server1");
    }

    virtual void TearDown() {
    }
};

TEST_F(TestCase_Pool, Attributes) {
    ASSERT_EQ(s->get_owner_pid(), c1->get_owner_pid());
    ASSERT_EQ(s->get_capacity(), c1->get_capacity());
    ASSERT_EQ(s->get_size(), c1->get_size());
    ASSERT_EQ(s->get_ref_count(), 2);
    ASSERT_EQ(s->get_ref_count(), c1->get_ref_count());
    ASSERT_EQ(s->get_owner_pid(), getpid());
    ASSERT_EQ(c1->get_owner_pid(), getpid());
}
TEST_F(TestCase_Pool, Attach1) {
    std::shared_ptr<Client> client = std::make_shared<Client>("server1");
    ASSERT_EQ(s->get_owner_pid(), client->get_owner_pid());
    ASSERT_EQ(s->get_ref_count(), client->get_ref_count());
    ASSERT_EQ(s->get_ref_count(), 3);
}

TEST_F(TestCase_Pool, Attach_N_Detach) {
    std::shared_ptr<Client> client2 = std::make_shared<Client>("server1");
    std::shared_ptr<Client> client3 = std::make_shared<Client>("server1");
    {
        std::shared_ptr<Client> client4 = std::make_shared<Client>("server1");
        ASSERT_EQ(s->get_ref_count(), client3->get_ref_count());
        ASSERT_EQ(s->get_ref_count(), client2->get_ref_count());
        ASSERT_EQ(s->get_ref_count(), c1->get_ref_count());
        ASSERT_EQ(s->get_ref_count(), 5);
        ASSERT_EQ(s->get_client_ids().size(), 4);

        ASSERT_EQ(s->get_client_ids()[0], c1->get_id());
        ASSERT_EQ(s->get_client_ids()[1], client2->get_id());
        ASSERT_EQ(s->get_client_ids()[2], client3->get_id());
        ASSERT_EQ(s->get_client_ids()[3], client4->get_id());
    }
    ASSERT_EQ(s->get_ref_count(), 4);
    ASSERT_EQ(s->get_client_ids().size(), 3);
    ASSERT_EQ(s->get_client_ids()[0], c1->get_id());
    ASSERT_EQ(s->get_client_ids()[1], client2->get_id());
    ASSERT_EQ(s->get_client_ids()[2], client3->get_id());
}

TEST_F(TestCase_Pool, Detach) {
    ASSERT_EQ(s->get_ref_count(), 2);
    ASSERT_EQ(c1->get_ref_count(), s->get_ref_count());
    ASSERT_EQ(s->get_client_ids().size(), 1);
    ASSERT_EQ(s->get_client_ids()[0], c1->get_id());
}

TEST_F(TestCase_Pool, ServerDetachFirst) {
    s.reset();
    ASSERT_EQ(c1->get_id(), 2);
    ASSERT_EQ(c1->get_status(), pool_status::POOL_DT);
    ASSERT_THROW(c1->get_ref_count(), pool_error);
    ASSERT_THROW(c1->get_name(), pool_error);
    ASSERT_THROW(c1->get_size(), pool_error);
    ASSERT_THROW(c1->get_capacity(), pool_error);
}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}