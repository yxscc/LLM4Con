# Evidence: SYZBOT-483d6c9b9231ea7e  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in p9_conn_cancel / p9_poll_workfn (9)
- mainline fix commit: `fbc0283fbeae27b88448c90305e738991457fee2`
- candidate JSON: `A_SYZBOT-483d6c9b9231ea7e.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "p9_conn_cancel", "fn_b": "p9_poll_workfn"}`

## Commit message

```
fbc0283fbeae27b88448c90305e738991457fee2
9p/trans_fd: mark concurrent read and writes to p9_conn->err

Writes for the error value of a connection are spinlock-protected inside
p9_conn_cancel, but lockless reads are present elsewhere to avoid
performing unnecessary work after an error has been met.

Mark the write and lockless reads to make KCSAN happy. Mark the write as
exclusive following the recommendation in "Lock-Protected Writes with
Lockless Reads" in tools/memory-model/Documentation/access-marking.txt
while we are at it.

Mark p9_fd_request and p9_conn_cancel m->err reads despite the fact that
they do not race with concurrent writes for stylistic reasons.

Reported-by: syzbot+d69a7cc8c683c2cb7506@syzkaller.appspotmail.com
Reported-by: syzbot+483d6c9b9231ea7e1851@syzkaller.appspotmail.com
Signed-off-by: Ignacio Encinas <ignacio@iencinas.com>
Message-ID: <20250318-p9_conn_err_benign_data_race-v3-1-290bb18335cc@iencinas.com>
Signed-off-by: Dominique Martinet <asmadeus@codewreck.org>
```

## CVE / bug description

```
KCSAN: data-race in p9_conn_cancel / p9_poll_workfn (9)
```

## Full mainline patch

```diff
commit fbc0283fbeae27b88448c90305e738991457fee2
Author: Ignacio Encinas <ignacio@iencinas.com>
Date:   Tue Mar 18 22:39:02 2025 +0100

    9p/trans_fd: mark concurrent read and writes to p9_conn->err
    
    Writes for the error value of a connection are spinlock-protected inside
    p9_conn_cancel, but lockless reads are present elsewhere to avoid
    performing unnecessary work after an error has been met.
    
    Mark the write and lockless reads to make KCSAN happy. Mark the write as
    exclusive following the recommendation in "Lock-Protected Writes with
    Lockless Reads" in tools/memory-model/Documentation/access-marking.txt
    while we are at it.
    
    Mark p9_fd_request and p9_conn_cancel m->err reads despite the fact that
    they do not race with concurrent writes for stylistic reasons.
    
    Reported-by: syzbot+d69a7cc8c683c2cb7506@syzkaller.appspotmail.com
    Reported-by: syzbot+483d6c9b9231ea7e1851@syzkaller.appspotmail.com
    Signed-off-by: Ignacio Encinas <ignacio@iencinas.com>
    Message-ID: <20250318-p9_conn_err_benign_data_race-v3-1-290bb18335cc@iencinas.com>
    Signed-off-by: Dominique Martinet <asmadeus@codewreck.org>

diff --git a/net/9p/trans_fd.c b/net/9p/trans_fd.c
index 2fea50bf047a..339ec4e54778 100644
--- a/net/9p/trans_fd.c
+++ b/net/9p/trans_fd.c
@@ -192,12 +192,13 @@ static void p9_conn_cancel(struct p9_conn *m, int err)
 
 	spin_lock(&m->req_lock);
 
-	if (m->err) {
+	if (READ_ONCE(m->err)) {
 		spin_unlock(&m->req_lock);
 		return;
 	}
 
-	m->err = err;
+	WRITE_ONCE(m->err, err);
+	ASSERT_EXCLUSIVE_WRITER(m->err);
 
 	list_for_each_entry_safe(req, rtmp, &m->req_list, req_list) {
 		list_move(&req->req_list, &cancel_list);
@@ -284,7 +285,7 @@ static void p9_read_work(struct work_struct *work)
 
 	m = container_of(work, struct p9_conn, rq);
 
-	if (m->err < 0)
+	if (READ_ONCE(m->err) < 0)
 		return;
 
 	p9_debug(P9_DEBUG_TRANS, "start mux %p pos %zd\n", m, m->rc.offset);
@@ -451,7 +452,7 @@ static void p9_write_work(struct work_struct *work)
 
 	m = container_of(work, struct p9_conn, wq);
 
-	if (m->err < 0) {
+	if (READ_ONCE(m->err) < 0) {
 		clear_bit(Wworksched, &m->wsched);
 		return;
 	}
@@ -623,7 +624,7 @@ static void p9_poll_mux(struct p9_conn *m)
 	__poll_t n;
 	int err = -ECONNRESET;
 
-	if (m->err < 0)
+	if (READ_ONCE(m->err) < 0)
 		return;
 
 	n = p9_fd_poll(m->client, NULL, &err);
@@ -666,6 +667,7 @@ static void p9_poll_mux(struct p9_conn *m)
 static int p9_fd_request(struct p9_client *client, struct p9_req_t *req)
 {
 	__poll_t n;
+	int err;
 	struct p9_trans_fd *ts = client->trans;
 	struct p9_conn *m = &ts->conn;
 
@@ -674,9 +676,10 @@ static int p9_fd_request(struct p9_client *client, struct p9_req_t *req)
 
 	spin_lock(&m->req_lock);
 
-	if (m->err < 0) {
+	err = READ_ONCE(m->err);
+	if (err < 0) {
 		spin_unlock(&m->req_lock);
-		return m->err;
+		return err;
 	}
 
 	WRITE_ONCE(req->status, REQ_STATUS_UNSENT);

```

## File: `net/9p/trans_fd.c`

- changed old-lines: [195, 200, 287, 454, 626, 677, 679]

### Function `p9_conn_cancel` (L186–L220, 35 lines)

```c
static void p9_conn_cancel(struct p9_conn *m, int err)
{
	struct p9_req_t *req, *rtmp;
	LIST_HEAD(cancel_list);

	p9_debug(P9_DEBUG_ERROR, "mux %p err %d\n", m, err);

	spin_lock(&m->req_lock);

	if (m->err) {
		spin_unlock(&m->req_lock);
		return;
	}

	m->err = err;

	list_for_each_entry_safe(req, rtmp, &m->req_list, req_list) {
		list_move(&req->req_list, &cancel_list);
		WRITE_ONCE(req->status, REQ_STATUS_ERROR);
	}
	list_for_each_entry_safe(req, rtmp, &m->unsent_req_list, req_list) {
		list_move(&req->req_list, &cancel_list);
		WRITE_ONCE(req->status, REQ_STATUS_ERROR);
	}

	spin_unlock(&m->req_lock);

	list_for_each_entry_safe(req, rtmp, &cancel_list, req_list) {
		p9_debug(P9_DEBUG_ERROR, "call back req %p\n", req);
		list_del(&req->req_list);
		if (!req->t_err)
			req->t_err = err;
		p9_client_cb(m->client, req, REQ_STATUS_ERROR);
	}
}
```

_(no external call sites found for `p9_conn_cancel`)_

### Function `p9_read_work` (L279–L409, 131 lines)

```c
static void p9_read_work(struct work_struct *work)
{
	__poll_t n;
	int err;
	struct p9_conn *m;

	m = container_of(work, struct p9_conn, rq);

	if (m->err < 0)
		return;

	p9_debug(P9_DEBUG_TRANS, "start mux %p pos %zd\n", m, m->rc.offset);

	if (!m->rc.sdata) {
		m->rc.sdata = m->tmp_buf;
		m->rc.offset = 0;
		m->rc.capacity = P9_HDRSZ; /* start by reading header */
	}

	clear_bit(Rpending, &m->wsched);
	p9_debug(P9_DEBUG_TRANS, "read mux %p pos %zd size: %zd = %zd\n",
		 m, m->rc.offset, m->rc.capacity,
		 m->rc.capacity - m->rc.offset);
	err = p9_fd_read(m->client, m->rc.sdata + m->rc.offset,
			 m->rc.capacity - m->rc.offset);
	p9_debug(P9_DEBUG_TRANS, "mux %p got %d bytes\n", m, err);
	if (err == -EAGAIN)
		goto end_clear;

	if (err <= 0)
		goto error;

	m->rc.offset += err;

	/* header read in */
	if ((!m->rreq) && (m->rc.offset == m->rc.capacity)) {
		p9_debug(P9_DEBUG_TRANS, "got new header\n");

		/* Header size */
		m->rc.size = P9_HDRSZ;
		err = p9_parse_header(&m->rc, &m->rc.size, NULL, NULL, 0);
		if (err) {
			p9_debug(P9_DEBUG_ERROR,
				 "error parsing header: %d\n", err);
			goto error;
		}

		p9_debug(P9_DEBUG_TRANS,
			 "mux %p pkt: size: %d bytes tag: %d\n",
			 m, m->rc.size, m->rc.tag);

		m->rreq = p9_tag_lookup(m->client, m->rc.tag);
		if (!m->rreq || (m->rreq->status != REQ_STATUS_SENT)) {
			p9_debug(P9_DEBUG_ERROR, "Unexpected packet tag %d\n",
				 m->rc.tag);
			err = -EIO;
			goto error;
		}

		if (m->rc.size > m->rreq->rc.capacity) {
			p9_debug(P9_DEBUG_ERROR,
				 "requested packet size too big: %d for tag %d with capacity %zd\n",
				 m->rc.size, m->rc.tag, m->rreq->rc.capacity);
			err = -EIO;
			goto error;
		}

		if (!m->rreq->rc.sdata) {
			p9_debug(P9_DEBUG_ERROR,
				 "No recv fcall for tag %d (req %p), disconnecting!\n",
				 m->rc.tag, m->rreq);
			p9_req_put(m->client, m->rreq);
			m->rreq = NULL;
			err = -EIO;
			goto error;
		}
		m->rc.sdata = m->rreq->rc.sdata;
		memcpy(m->rc.sdata, m->tmp_buf, m->rc.capacity);
		m->rc.capacity = m->rc.size;
	}

	/* packet is read in
	 * not an else because some packets (like clunk) have no payload
	 */
	if ((m->rreq) && (m->rc.offset == m->rc.capacity)) {
		p9_debug(P9_DEBUG_TRANS, "got new packet\n");
		m->rreq->rc.size = m->rc.offset;
		spin_lock(&m->req_lock);
		if (m->rreq->status == REQ_STATUS_SENT) {
			list_del(&m->rreq->req_list);
			p9_client_cb(m->client, m->rreq, REQ_STATUS_RCVD);
		} else if (m->rreq->status == REQ_STATUS_FLSHD) {
			/* Ignore replies associated with a cancelled request. */
			p9_debug(P9_DEBUG_TRANS,
				 "Ignore replies associated with a cancelled request\n");
		} else {
			spin_unlock(&m->req_lock);
			p9_debug(P9_DEBUG_ERROR,
				 "Request tag %d errored out while we were reading the reply\n",
				 m->rc.tag);
			err = -EIO;
			goto error;
		}
		spin_unlock(&m->req_lock);
		m->rc.sdata = NULL;
		m->rc.offset = 0;
		m->rc.capacity = 0;
		p9_req_put(m->client, m->rreq);
		m->rreq = NULL;
	}

end_clear:
	clear_bit(Rworksched, &m->wsched);

	if (!list_empty(&m->req_list)) {
		if (test_and_clear_bit(Rpending, &m->wsched))
			n = EPOLLIN;
		else
			n = p9_fd_poll(m->client, NULL, NULL);

		if ((n & EPOLLIN) && !test_and_set_bit(Rworksched, &m->wsched)) {
			p9_debug(P9_DEBUG_TRANS, "sched read work %p\n", m);
			schedule_work(&m->rq);
		}
	}

	return;
error:
	p9_conn_cancel(m, err);
	clear_bit(Rworksched, &m->wsched);
}
```

_(no external call sites found for `p9_read_work`)_

### Function `p9_write_work` (L445–L525, 81 lines)

```c
static void p9_write_work(struct work_struct *work)
{
	__poll_t n;
	int err;
	struct p9_conn *m;
	struct p9_req_t *req;

	m = container_of(work, struct p9_conn, wq);

	if (m->err < 0) {
		clear_bit(Wworksched, &m->wsched);
		return;
	}

	if (!m->wsize) {
		spin_lock(&m->req_lock);
		if (list_empty(&m->unsent_req_list)) {
			clear_bit(Wworksched, &m->wsched);
			spin_unlock(&m->req_lock);
			return;
		}

		req = list_entry(m->unsent_req_list.next, struct p9_req_t,
			       req_list);
		WRITE_ONCE(req->status, REQ_STATUS_SENT);
		p9_debug(P9_DEBUG_TRANS, "move req %p\n", req);
		list_move_tail(&req->req_list, &m->req_list);

		m->wbuf = req->tc.sdata;
		m->wsize = req->tc.size;
		m->wpos = 0;
		p9_req_get(req);
		m->wreq = req;
		spin_unlock(&m->req_lock);
	}

	p9_debug(P9_DEBUG_TRANS, "mux %p pos %d size %d\n",
		 m, m->wpos, m->wsize);
	clear_bit(Wpending, &m->wsched);
	err = p9_fd_write(m->client, m->wbuf + m->wpos, m->wsize - m->wpos);
	p9_debug(P9_DEBUG_TRANS, "mux %p sent %d bytes\n", m, err);
	if (err == -EAGAIN)
		goto end_clear;


	if (err < 0)
		goto error;
	else if (err == 0) {
		err = -EREMOTEIO;
		goto error;
	}

	m->wpos += err;
	if (m->wpos == m->wsize) {
		m->wpos = m->wsize = 0;
		p9_req_put(m->client, m->wreq);
		m->wreq = NULL;
	}

end_clear:
	clear_bit(Wworksched, &m->wsched);

	if (m->wsize || !list_empty(&m->unsent_req_list)) {
		if (test_and_clear_bit(Wpending, &m->wsched))
			n = EPOLLOUT;
		else
			n = p9_fd_poll(m->client, NULL, NULL);

		if ((n & EPOLLOUT) &&
		   !test_and_set_bit(Wworksched, &m->wsched)) {
			p9_debug(P9_DEBUG_TRANS, "sched write work %p\n", m);
			schedule_work(&m->wq);
		}
	}

	return;

error:
	p9_conn_cancel(m, err);
	clear_bit(Wworksched, &m->wsched);
}
```

_(no external call sites found for `p9_write_work`)_

### Function `p9_poll_mux` (L621–L653, 33 lines)

```c
static void p9_poll_mux(struct p9_conn *m)
{
	__poll_t n;
	int err = -ECONNRESET;

	if (m->err < 0)
		return;

	n = p9_fd_poll(m->client, NULL, &err);
	if (n & (EPOLLERR | EPOLLHUP | EPOLLNVAL)) {
		p9_debug(P9_DEBUG_TRANS, "error mux %p err %d\n", m, n);
		p9_conn_cancel(m, err);
	}

	if (n & EPOLLIN) {
		set_bit(Rpending, &m->wsched);
		p9_debug(P9_DEBUG_TRANS, "mux %p can read\n", m);
		if (!test_and_set_bit(Rworksched, &m->wsched)) {
			p9_debug(P9_DEBUG_TRANS, "sched read work %p\n", m);
			schedule_work(&m->rq);
		}
	}

	if (n & EPOLLOUT) {
		set_bit(Wpending, &m->wsched);
		p9_debug(P9_DEBUG_TRANS, "mux %p can write\n", m);
		if ((m->wsize || !list_empty(&m->unsent_req_list)) &&
		    !test_and_set_bit(Wworksched, &m->wsched)) {
			p9_debug(P9_DEBUG_TRANS, "sched write work %p\n", m);
			schedule_work(&m->wq);
		}
	}
}
```

_(no external call sites found for `p9_poll_mux`)_

### Function `p9_fd_request` (L666–L695, 30 lines)

```c
static int p9_fd_request(struct p9_client *client, struct p9_req_t *req)
{
	__poll_t n;
	struct p9_trans_fd *ts = client->trans;
	struct p9_conn *m = &ts->conn;

	p9_debug(P9_DEBUG_TRANS, "mux %p task %p tcall %p id %d\n",
		 m, current, &req->tc, req->tc.id);

	spin_lock(&m->req_lock);

	if (m->err < 0) {
		spin_unlock(&m->req_lock);
		return m->err;
	}

	WRITE_ONCE(req->status, REQ_STATUS_UNSENT);
	list_add_tail(&req->req_list, &m->unsent_req_list);
	spin_unlock(&m->req_lock);

	if (test_and_clear_bit(Wpending, &m->wsched))
		n = EPOLLOUT;
	else
		n = p9_fd_poll(m->client, NULL, NULL);

	if (n & EPOLLOUT && !test_and_set_bit(Wworksched, &m->wsched))
		schedule_work(&m->wq);

	return 0;
}
```

_(no external call sites found for `p9_fd_request`)_
