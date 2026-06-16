import { sql } from "../../lib/db.js";
import {
  requireUser,
  toLicense,
  getLicensesForUser,
  getLicenseStats,
} from "../../lib/auth.js";
import { parseBody, sendJson } from "../../lib/utils.js";

export default async function handler(req, res) {
  try {
    const user = await requireUser(req, res);
    if (!user) return;

    const id = req.query.id;
    if (!id) {
      return sendJson(res, 400, { success: false, message: "License id is required." });
    }

    if (req.method === "PATCH") {
      const body = await parseBody(req);
      const action = String(body.action || "");

      const existing = await sql`
        SELECT * FROM licenses
        WHERE id = ${id} AND user_id = ${user.id}
        LIMIT 1
      `;

      if (!existing.rows.length) {
        return sendJson(res, 404, { success: false, message: "License not found." });
      }

      const license = existing.rows[0];
      let nextStatus = license.status;

      if (action === "toggle-used") {
        if (license.status === "banned") {
          return sendJson(res, 400, { success: false, message: "Unban the license first." });
        }
        nextStatus = license.status === "used" ? "unused" : "used";
      } else if (action === "ban") {
        nextStatus = license.status === "banned" ? "unused" : "banned";
      } else {
        return sendJson(res, 400, { success: false, message: "Invalid action." });
      }

      await sql`
        UPDATE licenses
        SET status = ${nextStatus}
        WHERE id = ${id} AND user_id = ${user.id}
      `;

      const licenses = await getLicensesForUser(user.id);
      const stats = await getLicenseStats(user.id);

      return sendJson(res, 200, {
        success: true,
        message: "License updated.",
        license: toLicense({ ...license, status: nextStatus }),
        licenses,
        stats,
      });
    }

    if (req.method === "DELETE") {
      const deleted = await sql`
        DELETE FROM licenses
        WHERE id = ${id} AND user_id = ${user.id}
        RETURNING id
      `;

      if (!deleted.rows.length) {
        return sendJson(res, 404, { success: false, message: "License not found." });
      }

      const licenses = await getLicensesForUser(user.id);
      const stats = await getLicenseStats(user.id);

      return sendJson(res, 200, {
        success: true,
        message: "License deleted.",
        licenses,
        stats,
      });
    }

    return sendJson(res, 405, { success: false, message: "Method not allowed" });
  } catch (error) {
    sendJson(res, 500, { success: false, message: error.message || "License update failed." });
  }
}
