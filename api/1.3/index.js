import { sql } from "../../lib/db.js";
import {
  getUserByOwnerId,
  createClientSession,
  verifyClientSession,
} from "../../lib/auth.js";
import { parseBody, sendJson } from "../../lib/utils.js";

export default async function handler(req, res) {
  if (req.method !== "POST") {
    return sendJson(res, 405, { success: false, message: "Method not allowed" });
  }

  try {
    const body = await parseBody(req);
    const type = String(body.type || "").toLowerCase();

    if (type === "init") {
      const name = String(body.name || "").trim();
      const ownerid = String(body.ownerid || "").trim();
      const version = String(body.ver || body.version || "1.0").trim();

      const user = await getUserByOwnerId(ownerid);
      if (!user || user.app_name !== name) {
        return sendJson(res, 200, { success: false, message: "Invalid application credentials." });
      }

      if ((user.app_version || "1.0") !== version) {
        return sendJson(res, 200, {
          success: false,
          message: "Invalid version. Expected " + (user.app_version || "1.0"),
        });
      }

      const sessionid = createClientSession(user);
      return sendJson(res, 200, {
        success: true,
        message: "Initialized",
        sessionid,
        appinfo: {
          numUsers: "N/A",
          numOnlineUsers: "N/A",
          numKeys: "N/A",
          version: user.app_version || "1.0",
          customerPanelLink: "https://keyauth.cc/panel/",
        },
        newSession: true,
        nonce: Math.floor(Math.random() * 1e9).toString(),
      });
    }

    if (type === "license") {
      const key = String(body.key || "").trim();
      const sessionid = String(body.sessionid || "").trim();
      const ownerid = String(body.ownerid || "").trim();

      if (!key) {
        return sendJson(res, 200, { success: false, message: "No license key provided." });
      }

      const session = verifyClientSession(sessionid);
      if (!session || session.ownerId !== ownerid) {
        return sendJson(res, 200, { success: false, message: "Invalid session. Call init first." });
      }

      const user = await getUserByOwnerId(ownerid);
      if (!user) {
        return sendJson(res, 200, { success: false, message: "Invalid application." });
      }

      const found = await sql`
        SELECT * FROM licenses
        WHERE user_id = ${user.id} AND license_key = ${key}
        LIMIT 1
      `;

      if (!found.rows.length) {
        return sendJson(res, 200, { success: false, message: "Invalid license key." });
      }

      const license = found.rows[0];
      if (license.status === "banned") {
        return sendJson(res, 200, { success: false, message: "License is banned." });
      }

      if (license.status === "unused") {
        await sql`
          UPDATE licenses
          SET status = 'used'
          WHERE id = ${license.id}
        `;
      }

      return sendJson(res, 200, {
        success: true,
        message: "Logged in!",
        info: {
          username: key,
          subscriptions: [
            {
              subscription: "default",
              key: key,
              expiry: license.expiry_label,
              timeleft: license.expiry_label,
              level: String(license.level),
            },
          ],
          ip: req.headers["x-forwarded-for"] || req.socket?.remoteAddress || "unknown",
          hwid: body.hwid || null,
          createdate: new Date(license.created_at).toISOString(),
          lastlogin: new Date().toISOString(),
        },
        nonce: Math.floor(Math.random() * 1e9).toString(),
      });
    }

    return sendJson(res, 200, { success: false, message: "Unknown request type." });
  } catch (error) {
    sendJson(res, 500, { success: false, message: error.message || "Client API error." });
  }
}
